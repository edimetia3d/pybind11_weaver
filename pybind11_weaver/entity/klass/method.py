import collections
import logging

from typing import List, Tuple, Dict, Optional

import pylibclang._C
from pylibclang import cindex

from pybind11_weaver.utils import fn, common
from pybind11_weaver.entity import entity_base
from . import klass

_logger = logging.getLogger(__name__)

_def_bind_method = """
virtual const char * AddMethod_{method_identifier}(){{
    {bind_code}
}}
"""
_call_bind_method = """AddMethod_{method_identifier}();"""


class Method:

    def __init__(self, fn_cursor: cindex.Cursor, inject_docstring: bool, bind_name: str, identifier_name: str,
                 disable_mark: str):
        self.inect_docstring = inject_docstring
        self.fn_cursor = fn_cursor
        self.bind_name = bind_name
        self.identifier_name = identifier_name
        self.disable_mark = disable_mark

    def get_def_stmt(self, pybind11_obj_sym: str):
        fn_ptr = fn.get_fn_value_expr(self.fn_cursor)
        disable_bind = f"#define {self.disable_mark}" if fn_ptr is None else ""
        comment = self.fn_cursor.raw_comment
        should_add = self.inect_docstring and comment is not None
        comment = f'R"_pb11_weaver({comment})_pb11_weaver"' if comment else "nullptr"

        bind_code = f"""
const char * _pb11_weaver_comment_str = {comment};
{disable_bind}
#ifndef {self.disable_mark}
{pybind11_obj_sym}.{self.get_def_type()}(\"{self.bind_name}\",{fn_ptr}{',_pb11_weaver_comment_str' if should_add else ''});
#endif
return _pb11_weaver_comment_str;
"""
        return _def_bind_method.format(method_identifier=self.identifier_name, bind_code=bind_code)

    def get_call_stmt(self):
        return _call_bind_method.format(method_identifier=self.identifier_name)

    def get_def_type(self):
        if self.fn_cursor.is_static_method():
            return "def_static"
        else:
            return "def"


def is_explicit_instantiation(cursor: cindex.Cursor):
    source_file = cursor.location.file
    source_line = cursor.location.line
    # read the source_line-th line's content in the source file
    with open(source_file.name, 'r') as f:
        source_code = f.read()
        line_content = source_code.splitlines()[source_line - 1]

    return 'template class' in line_content or 'template struct' in line_content


def _get_template_param_arg_pair(template_cursor: cindex.Cursor, specialized_cursor: cindex.Cursor) -> Optional[
    List[str]]:
    decls = []
    index = 0
    for cursor in template_cursor.get_children():
        if cursor.kind == cindex.CursorKind.CXCursor_TemplateTypeParameter:
            assert specialized_cursor.get_template_argument_kind(
                index) == cindex.TemplateArgumentKind.CXTemplateArgumentKind_Type
            decls.append(f"using {cursor.spelling} = {specialized_cursor.get_template_argument_type(index).spelling};")
            index += 1
        elif cursor.kind == cindex.CursorKind.CXCursor_NonTypeTemplateParameter:
            if specialized_cursor.get_template_argument_kind(
                    index) == cindex.TemplateArgumentKind.CXTemplateArgumentKind_Integral:
                decls.append(
                    f"static constexpr int {cursor.spelling} = {specialized_cursor.get_template_argument_value(index)};")
                index += 1
            else:
                _logger.warning("Only Type and int template parameter supported for now")
                return None
    return decls


class GenMethod:

    def __init__(self, kls_entity: "klass.KlassEntity"):
        self.kls_entity = kls_entity
        self.added_method: Dict[str, int] = collections.defaultdict(int)

    @staticmethod
    def is_virtual(cursor: cindex.Cursor):
        if cursor.is_virtual_method() or cursor.is_pure_virtual_method():
            _logger.info(
                f"virtual method {fn.fn_ref_name(cursor)} at at {cursor.location.file}:{cursor.location.line} is not fully supported yet.")
            return True
        return False

    def run(self, pybind11_obj_sym: str) -> Tuple[List[str], List[str]]:
        """Return [binding_codes,extra_codes]"""
        codes = []
        extra_codes: List[str] = []
        kls_entity = self.kls_entity
        methods: Dict[str, Method] = dict()

        root_cursor = kls_entity.cursor
        if common.is_concreate_template(kls_entity.cursor):
            template_cursor = pylibclang._C.clang_getSpecializedCursorTemplate(kls_entity.cursor)
            template_cursor._tu = kls_entity.cursor._tu  # keep compatible with cindex and keep tu alive
            using_decls = _get_template_param_arg_pair(template_cursor, kls_entity.cursor)
            if using_decls is None:
                return [], []
            else:
                extra_codes.append("\n".join(using_decls))
            if root_cursor.location.file.name == kls_entity.gu.unsaved_file[0] or is_explicit_instantiation(
                    root_cursor):
                root_cursor = template_cursor
        for cursor in root_cursor.get_children():
            if cursor.kind == cindex.CursorKind.CXCursor_CXXMethod and kls_entity.could_member_export(
                    cursor) and not common.is_operator_overload(cursor):
                bind_name = fn.fn_python_name(cursor)
                unique_name = bind_name
                if self.added_method[bind_name] != 0:
                    unique_name += str(self.added_method[bind_name])
                self.added_method[bind_name] += 1
                self.is_virtual(cursor)  # print warning
                disable_mark = f"PB11_WEAVER_DISABLE_{self.kls_entity.get_pb11weaver_struct_name()}_{unique_name}"
                methods[unique_name] = Method(cursor, kls_entity.gu.io_config.gen_docstring, bind_name, unique_name,
                                              disable_mark)

        call_method_bind = []
        method_bind_body = []
        for _, method in methods.items():
            call_method_bind.append(method.get_call_stmt())
            method_bind_body.append(method.get_def_stmt(pybind11_obj_sym))

        codes.extend(call_method_bind)
        extra_codes.extend(method_bind_body)
        return codes, extra_codes
