from clang import cindex

from . import scope_list


def fn_arg_type(cursor: cindex.Cursor):
    return [param.type.spelling for param in cursor.get_arguments()]


def fn_ret_type(cursor: cindex.Cursor):
    return cursor.result_type.spelling


def get_fn_pointer_type(cursor: cindex.Cursor):
    if cursor.kind == cindex.CursorKind.CXX_METHOD and not cursor.is_static_method():
        return f"{fn_ret_type(cursor)} ({scope_list.get_full_qualified_name(cursor.semantic_parent)}::*)({','.join(fn_arg_type(cursor))})"
    else:
        return f"{fn_ret_type(cursor)} (*)({','.join(fn_arg_type(cursor))})"
