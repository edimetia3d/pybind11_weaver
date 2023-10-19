from typing import List

from clang import cindex

from . import scope_list


def fn_arg_type(cursor: cindex.Cursor) -> List[str]:
    return [param.type.get_canonical().spelling for param in cursor.get_arguments()]


def fn_ret_type(cursor: cindex.Cursor) -> str:
    return cursor.result_type.get_canonical().spelling


def _get_fn_pointer_type(cursor: cindex.Cursor) -> str:
    if cursor.kind == cindex.CursorKind.CXX_METHOD and not cursor.is_static_method():
        const_mark = "const" if cursor.is_const_method() else ""
        return f"{fn_ret_type(cursor)} ({scope_list.get_full_qualified_name(cursor.semantic_parent)}::*)({','.join(fn_arg_type(cursor))}) {const_mark}"
    else:
        return f"{fn_ret_type(cursor)} (*)({','.join(fn_arg_type(cursor))})"


def get_fn_pointer(cursor: cindex.Cursor) -> str:
    pointer = f"&{scope_list.get_full_qualified_name(cursor)}"
    method_pointer_type = _get_fn_pointer_type(cursor)
    return f"static_cast<{method_pointer_type}>({pointer})"


__wrapped_db = set()


def get_wrapped_types():
    return __wrapped_db


def warp_type(type: cindex.Type, param_name: str):
    type = type.get_canonical()
    ret = None, param_name
    if type.kind == cindex.TypeKind.POINTER:
        pointee = type.get_pointee().get_canonical()
        # if pointee is a pointer, warp it
        if pointee.kind in [cindex.TypeKind.POINTER]:
            ret = f"pybind11_weaver::PointerWrapper<{type.spelling}>", param_name
        if pointee.kind in [cindex.TypeKind.FUNCTIONPROTO]:
            ret = f"std::function<{pointee.spelling}>", f"{param_name}.target<{pointee.spelling}>()"
        # if pointee is a incompelete type, warp it
        pointee_decl = pointee.get_declaration()
        if pointee_decl.kind in [cindex.CursorKind.STRUCT_DECL,
                                 cindex.CursorKind.CLASS_DECL] and not pointee_decl.is_definition():
            ret = f"pybind11_weaver::PointerWrapper<{type.spelling}>", param_name
    if ret[0] is not None:
        __wrapped_db.add(type.spelling)
    return ret


__warpper_template = """[]({params}){{
    return {ret_expr};
}}"""


def get_fn_wrapper(cursor: cindex.Cursor):
    params = []
    if cursor.kind != cindex.CursorKind.FUNCTION_DECL:
        params.append(f"{cursor.semantic_parent.spelling}& self")
    args = []
    warp = False
    for param in cursor.get_arguments():
        param_t = param.type.get_canonical()
        param_spelling = param.spelling
        if param_spelling == "":
            param_spelling = "arg" + str(len(args))
        warp_t, arg_use = warp_type(param_t, param_spelling)
        if warp_t:
            warp = True
            params.append(f"{warp_t} {param_spelling}")
        else:
            params.append(f"{param_t.spelling} {param_spelling}")
        args.append(arg_use)
    ret_t = cursor.result_type.get_canonical()
    ret_expr = f"{cursor.spelling}({','.join(args)})"
    if cursor.kind != cindex.CursorKind.FUNCTION_DECL:
        ret_expr = f"self.{ret_expr}"
    warp_t, _ = warp_type(ret_t, "")
    if warp_t:
        warp = True
        ret_expr = f"{warp_t}({ret_expr})"
    if warp:
        return __warpper_template.format(params=','.join(params), ret_expr=ret_expr)
    else:
        return None


def get_fn_value_expr(cursor: cindex.Cursor) -> str:
    wrapper = get_fn_wrapper(cursor)
    if wrapper:
        return wrapper
    else:
        return get_fn_pointer(cursor)
