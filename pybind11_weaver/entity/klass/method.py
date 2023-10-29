import collections
import logging

from typing import List, Tuple, Dict, Optional

import pylibclang._C
from pylibclang import cindex

from pybind11_weaver.utils import fn, common
from . import klass

_logger = logging.getLogger(__name__)

_def_bind_method = """
virtual const char * AddMethod_{method_identifier}(){{
    {bind_code}
}}
"""
_call_bind_method = """AddMethod_{method_identifier}();"""


def get_def_type(cursor):
    if cursor.is_static_method():
        return "def_static"
    else:
        return "def"


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
{pybind11_obj_sym}.{get_def_type(self.fn_cursor)}(\"{self.bind_name}\",{fn_ptr}{',_pb11_weaver_comment_str' if should_add else ''});
#endif
return _pb11_weaver_comment_str;
"""
        return _def_bind_method.format(method_identifier=self.identifier_name, bind_code=bind_code)

    def get_call_stmt(self):
        return _call_bind_method.format(method_identifier=self.identifier_name)


class GenMethod:

    def __init__(self, kls_entity: "klass.KlassEntity"):
        self.kls_entity = kls_entity
        self.added_method: Dict[str, List[Method]] = collections.defaultdict(list)

    def run(self, pybind11_obj_sym: str) -> Tuple[List[str], List[str]]:
        """Return [binding_codes,extra_codes]"""
        codes = []
        extra_codes: List[str] = []
        kls_entity = self.kls_entity
        methods = []

        root_cursor, using_decls, _ = common.get_def_cls_cursor(kls_entity.cursor)
        if using_decls is None:
            return [], []
        extra_codes.append("\n".join(using_decls))
        for cursor in root_cursor.get_children():
            if cursor.kind == cindex.CursorKind.CXCursor_CXXMethod and kls_entity.could_member_export(
                    cursor) and not common.is_operator_overload(cursor):
                bind_name = fn.fn_python_name(cursor)
                unique_name = bind_name
                while len(self.added_method[bind_name]) != 0:
                    if get_def_type(cursor) != get_def_type(self.added_method[bind_name][0].fn_cursor):
                        bind_name = bind_name + "_"
                        unique_name = bind_name
                        _logger.warning(
                            f"pybind11 does not support mix def and def_static overloading, bind {cursor.spelling} to {unique_name}")

                    else:
                        unique_name = bind_name + str(len(self.added_method[bind_name]))
                        break
                disable_mark = f"PB11_WEAVER_DISABLE_{self.kls_entity.get_pb11weaver_struct_name()}_{unique_name}"
                methods.append(Method(cursor, kls_entity.gu.io_config.gen_docstring, bind_name, unique_name,
                                      disable_mark))
                self.added_method[bind_name].append(methods[-1])

        call_method_bind = []
        method_bind_body = []
        for method in methods:
            call_method_bind.append(method.get_call_stmt())
            method_bind_body.append(method.get_def_stmt(pybind11_obj_sym))

        codes.extend(call_method_bind)
        extra_codes.extend(method_bind_body)
        return codes, extra_codes
