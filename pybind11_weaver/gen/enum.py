from typing import List

from clang import cindex

from pybind11_weaver import entity_base


class EnumEntity(entity_base.EntityBase):

    def __init__(self, cursor: cindex.Cursor):
        entity_base.EntityBase.__init__(self, cursor)
        assert cursor.kind == cindex.CursorKind.ENUM_DECL
        self.spelling = cursor.spelling
        self.elements: List[str] = self.__init_elements()
        self.scope = entity_base.ScopeList(cursor.type.spelling)

    def __init_elements(self):
        elements = []
        for cursor in self.cursor.get_children():
            elements.append(cursor.spelling)
        return elements
