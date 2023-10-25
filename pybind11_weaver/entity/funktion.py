from typing import List
from pylibclang import cindex

from . import entity_base
from pybind11_weaver.utils import fn, common, scope_list

from pybind11_weaver import gen_unit


class FunctionEntity(entity_base.Entity):

    def __init__(self, gu: gen_unit.GenUnit, cursor: cindex.Cursor):
        entity_base.Entity.__init__(self, gu, cursor)
        self.overloads: List[FunctionEntity] = []
        assert cursor.kind == cindex.CursorKind.CXCursor_FunctionDecl

    def get_cpp_struct_name(self) -> str:
        return self.qualified_name().replace("::", "_")

    def init_default_pybind11_value(self, parent_scope_sym: str) -> str:
        return f"static_cast<pybind11::module_&>({parent_scope_sym})"

    def update_this_func_stmts(self, pybind11_obj_sym: str) -> List[str]:
        code = []
        bind_name = common.get_safe_indentifier_name(self.cursor)
        code.append(
            f"{pybind11_obj_sym}.def(\"{bind_name}\",{fn.get_fn_value_expr(self.cursor)});")
        if self.gu.io_config.gen_docstring:
            code[-1] = entity_base._inject_docstring(code[-1], self.cursor, "last_arg")
        return code

    def update_stmts(self, pybind11_obj_sym: str) -> List[str]:
        code = self.update_this_func_stmts(pybind11_obj_sym)
        for overload in self.overloads:
            code += overload.update_stmts(pybind11_obj_sym)

        return code

    def default_pybind11_type_str(self) -> str:
        return f"pybind11::module_ &"
