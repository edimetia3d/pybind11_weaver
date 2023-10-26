from typing import Union
import functools

from pylibclang import cindex


def get_full_qualified_scopes(cursor: cindex.Cursor):
    if cursor.kind == cindex.CursorKind.CXCursor_TranslationUnit:
        return []
    values = []
    # extern C seems to be a scope with kind of CursorKind.UNEXPOSED_DECL
    cursor = cursor.semantic_parent
    while cursor is not None and cursor.kind not in [cindex.CursorKind.CXCursor_TranslationUnit,
                                                     cindex.CursorKind.CXCursor_UnexposedDecl]:
        values.append(cursor.displayname)
        cursor = cursor.semantic_parent
    values.reverse()
    return values


def get_full_qualified_name(cursor: cindex.Cursor, seperator="::", base_name=None):
    if base_name is None:
        base_name = cursor.displayname
    return seperator.join(get_full_qualified_scopes(cursor) + [base_name])
