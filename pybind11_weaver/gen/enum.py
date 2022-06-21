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

    def declare_expr(self, module_sym: str = "m"):
        type_full_name = self.cursor.type.spelling
        code = f"pybind11::enum_<{type_full_name}>({module_sym}, \"{self.cursor.spelling}\",pybind11::arithmetic())"
        for cursor in self.cursor.get_children():
            code += f".value(\"{cursor.spelling}\", {type_full_name}::{cursor.spelling})"
        return code
