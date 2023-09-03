from typing import List

from clang import cindex

from . import entity_base


class NamespaceEntity(entity_base.Entity):

    def __init__(self, cursor: cindex.Cursor):
        entity_base.Entity.__init__(self, cursor)
        assert cursor.kind == cindex.CursorKind.NAMESPACE

    def get_cpp_struct_name(self) -> str:
        return "_".join(self.get_scope() + [self.name])

    def create_pybind11_obj_expr(self, module_sym: str) -> str:
        code = f'{module_sym}.def_submodule("{self.name}")'
        return code

    def update_stmts(self, pybind11_obj_sym: str) -> List[str]:
        return []

    def pybind11_type_str(self) -> str:
        return f"pybind11::module_"
