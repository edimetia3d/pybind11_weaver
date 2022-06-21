from typing import List

from clang import cindex

from pybind11_weaver import entity_base


class EnumEntity(entity_base.EntityBase):

    def __init__(self, cursor: cindex.Cursor):
        entity_base.EntityBase.__init__(self, cursor)
        assert cursor.kind == cindex.CursorKind.ENUM_DECL

    def get_scope(self) -> entity_base.ScopeList:
        return entity_base.ScopeList(self.cursor)

    def get_spelling(self) -> str:
        return self.cursor.spelling

    def get_unique_name(self) -> str:
        return self.cursor.type.spelling

    def declare_expr(self, module_sym: str) -> str:
        code = f"{self.pybind11_type_str()}({module_sym}, \"{self.cursor.spelling}\",pybind11::arithmetic())"
        return code

    def update_stmts(self, sym: str) -> List[str]:
        type_full_name = self.cursor.type.spelling
        code = []
        for cursor in self.cursor.get_children():
            code.append(f"{sym}.value(\"{cursor.spelling}\", {type_full_name}::{cursor.spelling});")
        return code

    def pybind11_type_str(self) -> str:
        return f"pybind11::enum_<{self.cursor.type.spelling}>"
