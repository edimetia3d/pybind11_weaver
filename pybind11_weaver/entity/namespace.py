from typing import List

from pylibclang import cindex

from . import entity_base

from pybind11_weaver import gen_unit


class NamespaceEntity(entity_base.Entity):

    def __init__(self, gu: gen_unit.GenUnit, cursor: cindex.Cursor):
        entity_base.Entity.__init__(self, gu, cursor)
        assert cursor.kind == cindex.CursorKind.CXCursor_Namespace

    def get_cpp_struct_name(self) -> str:
        return "_".join(self.get_scope() + [self.name])

    def init_default_pybind11_value(self, parent_scope_sym: str) -> str:
        module = f"static_cast<pybind11::module_&>({parent_scope_sym})"
        code = f'{module}.def_submodule("{self.name}")'
        return code

    def update_stmts(self, pybind11_obj_sym: str) -> List[str]:
        return []

    def default_pybind11_type_str(self) -> str:
        return f"pybind11::module_"
