from typing import List

from clang import cindex

from pybind11_weaver import entity_base


class NamespaceEntity(entity_base.EntityBase):

    def __init__(self, cursor: cindex.Cursor):
        entity_base.EntityBase.__init__(self, cursor)
        assert cursor.kind == cindex.CursorKind.NAMESPACE

    def get_unique_name(self) -> str:
        return "::".join(self.get_scope().scopes + [self.get_spelling()])

    def declare_expr(self, module_sym: str) -> str:
        code = f'{module_sym}.def_submodule("{self.get_spelling()}")'
        return code

    def update_stmts(self, sym: str) -> List[str]:
        return []

    def pybind11_type_str(self) -> str:
        return f"pybind11::module_"
