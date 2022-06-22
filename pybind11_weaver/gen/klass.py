from typing import List

from clang import cindex

from pybind11_weaver import entity_base


class ClassEntity(entity_base.EntityBase):

    def __init__(self, cursor: cindex.Cursor):
        entity_base.EntityBase.__init__(self, cursor)
        assert cursor.kind in [cindex.CursorKind.CLASS_DECL, cindex.CursorKind.STRUCT_DECL]

    def get_unique_name(self) -> str:
        return self.cursor.type.spelling

    def declare_expr(self, module_sym: str) -> str:
        code = f'{self.pybind11_type_str()}({module_sym},"{self.get_spelling()}")'
        return code

    def update_stmts(self, sym: str) -> List[str]:
        return []

    def pybind11_type_str(self) -> str:
        type_full_name = self.cursor.type.spelling
        return f"pybind11::class_<{type_full_name}>"
