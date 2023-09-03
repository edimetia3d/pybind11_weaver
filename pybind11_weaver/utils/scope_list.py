from typing import Union
import functools

from clang import cindex


def get_full_qualified_scopes(cursor: cindex.Cursor):
    if cursor.kind == cindex.CursorKind.TRANSLATION_UNIT:
        return []
    values = []
    cursor = cursor.semantic_parent
    while cursor is not None and cursor.kind != cindex.CursorKind.TRANSLATION_UNIT:
        values.append(cursor.spelling)
        cursor = cursor.semantic_parent
    values.reverse()
    return values


def get_full_qualified_name(cursor: cindex.Cursor, seperator="::"):
    return seperator.join(get_full_qualified_scopes(cursor) + [cursor.spelling])
