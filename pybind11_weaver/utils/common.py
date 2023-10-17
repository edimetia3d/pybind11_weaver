from clang import cindex


def is_visible(cursor: cindex.Cursor):
    if cursor.kind not in [cindex.CursorKind.FUNCTION_DECL, cindex.CursorKind.CXX_METHOD]:
        return True  # only control visibility of function and method
    return cursor.linkage == cindex.LinkageKind.EXTERNAL
