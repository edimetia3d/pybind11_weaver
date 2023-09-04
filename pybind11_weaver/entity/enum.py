from typing import List

from clang import cindex

from . import entity_base


class EnumEntity(entity_base.Entity):

    def __init__(self, cursor: cindex.Cursor):
        entity_base.Entity.__init__(self, cursor)
        assert cursor.kind == cindex.CursorKind.ENUM_DECL

    def get_cpp_struct_name(self) -> str:
        return self.cursor.type.spelling.replace("::", "_")

    def init_default_pybind11_value(self, parent_scope_sym: str) -> str:
        code = f"{parent_scope_sym}, \"{self.name}\",pybind11::arithmetic()"
        return code

    def update_stmts(self, pybind11_obj_sym: str) -> List[str]:
        type_full_name = self.cursor.type.spelling
        code = []
        for cursor in self.cursor.get_children():
            code.append(f"{pybind11_obj_sym}.value(\"{cursor.spelling}\", {type_full_name}::{cursor.spelling});")
        return code

    def default_pybind11_type_str(self) -> str:
        return f"pybind11::enum_<{self.cursor.type.spelling}>"
