import abc
import functools
from typing import Union

from clang import cindex


class ScopeList:

    def __init__(self, v: Union[str, cindex.Cursor]):
        if isinstance(v, str):
            self.scopes = self.__get_from_full_qualified_name(v)
        else:
            assert isinstance(v, cindex.Cursor)
            self.scopes = self.__get_from_cursor(v)

    @staticmethod
    def __get_from_cursor(cursor: cindex.Cursor):
        values = []
        cursor = cursor.semantic_parent
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


class EntityBase(abc.ABC):

    def __init__(self, cursor: cindex.Cursor):
        self.cursor = cursor

    def get_scope(self) -> ScopeList:
        return ScopeList(self.cursor)

    def get_spelling(self) -> str:
        return self.cursor.spelling

    @abc.abstractmethod
    def get_unique_name(self) -> str:
        pass

    @abc.abstractmethod
    def declare_expr(self, module_sym: str) -> str:
        pass

    @abc.abstractmethod
    def update_stmts(self, sym: str) -> str:
        pass

    @abc.abstractmethod
    def pybind11_type_str(self) -> str:
        pass
