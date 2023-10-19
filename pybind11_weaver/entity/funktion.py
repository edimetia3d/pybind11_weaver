from typing import List
import functools

from clang import cindex

from . import entity_base
from pybind11_weaver.utils import fn

from pybind11_weaver import gen_unit


class FunctionEntity(entity_base.Entity):

    def __init__(self, gu: gen_unit.GenUnit, cursor: cindex.Cursor):
        entity_base.Entity.__init__(self, gu, cursor)
        self.overloads: List[cindex.Cursor] = []
        assert cursor.kind == cindex.CursorKind.FUNCTION_DECL

    def get_cpp_struct_name(self) -> str:
        return self.qualified_name().replace("::", "_")

    def init_default_pybind11_value(self, parent_scope_sym: str) -> str:
        return f"static_cast<pybind11::module_&>({parent_scope_sym})"

    def update_stmts(self, pybind11_obj_sym: str) -> List[str]:
        code = []
        targets = [self.cursor] + self.overloads
        for t in targets:
            code.append(
                f"{pybind11_obj_sym}.def(\"{self.name}\",{fn.get_fn_value_expr(t)});")
            if self.gu.io_config.gen_docstring:
                code[-1] = entity_base._inject_docstring(code[-1], t, "last_arg")
        return code

    def default_pybind11_type_str(self) -> str:
        return f"pybind11::module_ &"
