from typing import Union
import functools

from clang import cindex


def get_full_qualified_scopes(cursor: cindex.Cursor):
    if cursor.kind == cindex.CursorKind.TRANSLATION_UNIT:
        return []
    values = []
    # extern C seems to be a scope with kind of CursorKind.UNEXPOSED_DECL
    cursor = cursor.semantic_parent
    while cursor is not None and cursor.kind not in [cindex.CursorKind.TRANSLATION_UNIT,
                                                     cindex.CursorKind.UNEXPOSED_DECL]:
        values.append(cursor.spelling)
        cursor = cursor.semantic_parent
    values.reverse()
    return values


def get_full_qualified_name(cursor: cindex.Cursor, seperator="::"):
    return seperator.join(get_full_qualified_scopes(cursor) + [cursor.spelling])
