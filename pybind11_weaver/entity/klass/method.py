import collections
import logging

from typing import List, Tuple, Dict

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
        kls_entity = self.kls_entity
        methods: Dict[str, Method] = dict()
        for cursor in kls_entity.cursor.get_children():
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
        return codes, [bind_all]
