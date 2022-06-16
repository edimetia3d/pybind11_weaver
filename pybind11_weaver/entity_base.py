import functools

from clang import cindex


class ScopeList:

    def __init__(self, v):
        if isinstance(v, str):
            self.scopes = self.__get_from_full_qualified_name(v)
        else:
            assert isinstance(v, cindex.Cursor)
            self.scopes = self.__get_from_cursor(v)

    @staticmethod
    def __get_from_cursor(cursor: cindex.Cursor):
        values = []
        while cursor is not None and cursor.kind != cindex.CursorKind.TRANSLATION_UNIT:
            values.append(cursor.spelling)
            cursor = cursor.semantic_parent
        values.reverse()
        return values

    @staticmethod
    def __get_from_full_qualified_name(full_name: str):
        return full_name.split("::")

    @functools.lru_cache
    def str(self, seperator="::"):
        return seperator.join(self.scopes)


class EntityBase:

    def __init__(self, cursor: cindex.Cursor):
        self.cursor = cursor
