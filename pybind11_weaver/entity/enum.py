from typing import List

from pylibclang import cindex

from . import entity_base

from pybind11_weaver import gen_unit


class EnumEntity(entity_base.Entity):

    def __init__(self, gu: gen_unit.GenUnit, cursor: cindex.Cursor):
        entity_base.Entity.__init__(self, gu, cursor)
        assert cursor.kind == cindex.CursorKind.CXCursor_EnumDecl

    def reference_name(self) -> str:
        return self.cursor.type.spelling

    def get_pb11weaver_struct_name(self) -> str:
        return self.cursor.type.spelling.replace("::", "_")

    def init_default_pybind11_value(self, parent_scope_sym: str) -> str:
        code = f"{parent_scope_sym}, \"{self.name}\",pybind11::arithmetic()"
        if self.gu.io_config.gen_docstring:
            code = entity_base._inject_docstring(code, self.cursor, "append")
        return code

    def update_stmts(self, pybind11_obj_sym: str) -> List[str]:
        type_full_name = self.cursor.type.spelling
        code = []
        for cursor in self.cursor.get_children():
            code.append(f"{pybind11_obj_sym}.value(\"{cursor.spelling}\", {type_full_name}::{cursor.spelling});")
            if self.gu.io_config.gen_docstring:
                code[-1] = entity_base._inject_docstring(code[-1], cursor, "last_arg")
        return code

    def default_pybind11_type_str(self) -> str:
        return f"pybind11::enum_<{self.cursor.type.spelling}>"
