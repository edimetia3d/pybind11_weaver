from pylibclang import cindex
import pylibclang._C


def is_visible(cursor: cindex.Cursor):
    return pylibclang._C.clang_getCursorVisibility(cursor) == cindex._C.CXVisibilityKind.CXVisibility_Default
