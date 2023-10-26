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
virtual void BindMethod_{method_identifier}(Pybind11T & obj){{
    {bind_expr};
}}
"""
_call_bind_method = """BindMethod_{method_identifier}(obj);"""


class Method:

    def __init__(self, fn_cursor: cindex.Cursor, inject_docstring: bool, identifier_name: str):
        self.inect_docstring = inject_docstring
        self.fn_cursor = fn_cursor
        self.bind_name = fn.fn_python_name(fn_cursor)
        self.identifier_name = identifier_name

    def get_def_stmt(self):
        bind_expr = f"obj.{self.get_def_type()}(\"{self.bind_name}\",{fn.get_fn_value_expr(self.fn_cursor)})"
        if self.inect_docstring:
            bind_expr = entity_base._inject_docstring(
                bind_expr, self.fn_cursor, "last_arg")
        return _def_bind_method.format(method_identifier=self.identifier_name, bind_expr=bind_expr)

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
            _logger.warning(
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
            if is_explicit_instantiation(root_cursor):
                root_cursor = template_cursor
        for cursor in root_cursor.get_children():
            if cursor.kind == cindex.CursorKind.CXCursor_CXXMethod and kls_entity.is_pubic(
                    cursor) and not common.is_operator_overload(cursor):
                bind_name = fn.fn_python_name(cursor)
                unique_name = bind_name
                if self.added_method[bind_name] != 0:
                    unique_name += str(self.added_method[bind_name])
                self.added_method[bind_name] += 1
                self.is_virtual(cursor)  # print warning
                methods[unique_name] = Method(cursor, kls_entity.gu.io_config.gen_docstring, unique_name)

        call_method_bind = []
        method_bind_body = []
        for _, method in methods.items():
            call_method_bind.append(method.get_call_stmt())
            method_bind_body.append(method.get_def_stmt())
        new_line = "\n"
        bind_all = f"""
        void Pybind11WeaverBindAllMethods(Pybind11T & obj){{
           {new_line.join(call_method_bind)}
        }}

        {new_line.join(method_bind_body)}
"""

        codes.append(f"Pybind11WeaverBindAllMethods({pybind11_obj_sym});")
        extra_codes.append(bind_all)
        return codes, extra_codes
