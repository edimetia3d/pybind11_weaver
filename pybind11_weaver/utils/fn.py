from typing import List, Tuple

from pylibclang import cindex

from . import scope_list


def fn_arg_type(cursor: cindex.Cursor) -> List[str]:
    return [param.type.get_canonical().spelling for param in cursor.get_arguments()]


def fn_ret_type(cursor: cindex.Cursor) -> str:
    return cursor.result_type.get_canonical().spelling


def _get_fn_pointer_type(cursor: cindex.Cursor) -> str:
    """For libclang do not provide API to construct the pointer type, we have to construct it by ourself."""
    if cursor.kind == cindex.CursorKind.CXCursor_CXXMethod and not cursor.is_static_method():
        const_mark = "const" if cursor.is_const_method() else ""
        return f"{fn_ret_type(cursor)} ({scope_list.get_full_qualified_name(cursor.semantic_parent)}::*)({','.join(fn_arg_type(cursor))}) {const_mark}"
    else:
        return f"{fn_ret_type(cursor)} (*)({','.join(fn_arg_type(cursor))})"


_wrapped_db = set()


def get_wrapped_types():
    return _wrapped_db


class ValueCast:
    nest_level = -1

    def __init__(self, c_type: cindex.Type, cpp_type: str):
        self.c_type = c_type.get_canonical()
        self.cpp_type = cpp_type

    def __call__(self, value: str, **kwargs):
        self.nest_level += 1
        ret = self.cvt(value, **kwargs)
        self.nest_level -= 1
        return ret

    def cvt(self, value: str, **kwargs):
        pass


class CPointerToWrapped(ValueCast):

    def __init__(self, *args, **kwargs):
        ValueCast.__init__(self, *args, **kwargs)
        _wrapped_db.add(self.c_type.spelling)

    def cvt(self, c_expr: str, **kwargs):
        if "move_out" in kwargs and kwargs["move_out"]:
            return f"pybind11_weaver::WrapP<{self.c_type.spelling}>({c_expr}).release()"
        else:
            return f"pybind11_weaver::WrapP<{self.c_type.spelling}>({c_expr}).get()"


class WrappedToCPointer(ValueCast):
    def __init__(self, *args, **kwargs):
        ValueCast.__init__(self, *args, **kwargs)
        _wrapped_db.add(self.c_type.spelling)

    def cvt(self, cpp_expr: str, **kwargs):
        return f"({cpp_expr})->ptr"


class CFnPointerToCppStdFn(ValueCast):

    def cvt(self, c_expr: str, **kwargsk):
        assert self.c_type.kind == cindex.TypeKind.CXType_FunctionProto
        return wrap_c_function_to_cpp(f"({c_expr})", self.c_type.get_result(),
                                      [d for d in self.c_type.argument_types()],
                                      [f"arg{self.nest_level}_{i}" for i, _ in enumerate(self.c_type.argument_types())],
                                      force_warp=True)


class CppStdFnToCFnPointer(ValueCast):

    def cvt(self, cpp_expr: str, **kwargs):
        assert self.c_type.kind == cindex.TypeKind.CXType_FunctionProto
        return wrap_cpp_function_to_c(f"({cpp_expr})", self.c_type.get_result(),
                                      [d for d in self.c_type.argument_types()],
                                      [f"arg{self.nest_level}_{i}" for i, _ in enumerate(self.c_type.argument_types())])


def get_cpp_type(c_type: cindex.Type) -> Tuple[str, "CValueToCppValue", "CppValueToCValue"]:
    c_type = c_type.get_canonical()
    ret = c_type.spelling, lambda x: x, lambda x: x
    if c_type.kind == cindex.TypeKind.CXType_Pointer:
        pointee = c_type.get_pointee().get_canonical()
        if pointee.kind in [cindex.TypeKind.CXType_Pointer, cindex.TypeKind.CXType_Void]:
            cpp_type = f"pybind11_weaver::PointerWrapper<{c_type.spelling}> *"
            return cpp_type, CPointerToWrapped(c_type, cpp_type), WrappedToCPointer(c_type, cpp_type)
        if pointee.kind in [cindex.TypeKind.CXType_FunctionProto]:
            ret_t, args_t = c_function_sig_to_cpp_function_sig(pointee.get_result(), pointee.argument_types())
            cpp_type = f"std::function<{ret_t} ({','.join(args_t)})>"
            return cpp_type, CFnPointerToCppStdFn(pointee, cpp_type), CppStdFnToCFnPointer(pointee, cpp_type)
        # if pointee is a incompelete type, warp it
        pointee_decl = pointee.get_declaration()
        if pointee_decl.kind in [cindex.CursorKind.CXCursor_StructDecl,
                                 cindex.CursorKind.CXCursor_ClassDecl] and not pointee_decl.is_definition():
            cpp_type = f"pybind11_weaver::PointerWrapper<{c_type.spelling}> *"
            return cpp_type, CPointerToWrapped(c_type, cpp_type), WrappedToCPointer(c_type, cpp_type)

    return ret


__c_function_to_cpp_template = """[=]({params}){{
    return {ret_expr};
}}"""


def wrap_c_function_to_cpp(fn_name: str, ret_t: cindex.Type, args_t: List[cindex.Type], args_names: List[str],
                           cls_name: str = None, force_warp=False):
    # param and forwarded args
    new_params = []
    if cls_name:
        new_params.append(f"{cls_name}& self")
    forward_args = []
    wrap_param = False
    for param_t, param_spelling in zip(args_t, args_names):
        param_t = param_t.get_canonical()
        cpp_type, _, cpp_to_c = get_cpp_type(param_t)
        if cpp_type != param_t.spelling:
            wrap_param = True
            new_params.append(f"{cpp_type} {param_spelling}")
            forward_args.append(cpp_to_c(param_spelling))
        else:
            new_params.append(f"{param_t.spelling} {param_spelling}")
            forward_args.append(param_spelling)
    ret_expr = f"{fn_name}({','.join(forward_args)})"
    if cls_name:
        ret_expr = f"self.{ret_expr}"

    # ret
    ret_t = ret_t.get_canonical()
    cpp_type, c_to_cpp, _ = get_cpp_type(ret_t)
    wrap_ret = False
    if cpp_type != ret_t.get_canonical().spelling:
        wrap_ret = True
        ret_expr = c_to_cpp(ret_expr, move_out=True)
    if wrap_param or wrap_ret or force_warp:
        return __c_function_to_cpp_template.format(params=','.join(new_params), ret_expr=ret_expr)
    else:
        return None


__wraped_cpp_fn_uuid = 0
__cpp_function_to_c_template = """[]({params}){{
    auto &to_call = pybind11_weaver::FnPointerWrapper<{cpp_type_list}>::FnProxy({uuid});
    return {ret_expr};
}}"""


def c_function_sig_to_cpp_function_sig(c_ret_t, c_args_t) -> Tuple[str, List[str]]:
    ret_cpp_t = get_cpp_type(c_ret_t)[0]
    args_cpp_t = [get_cpp_type(a)[0] for a in c_args_t]
    return ret_cpp_t, args_cpp_t


def wrap_cpp_function_to_c(cpp_callable_name: str,
                           c_ret_t: cindex.Type,
                           c_args_t: List[cindex.Type],
                           c_args_names: List[str]):
    global __wraped_cpp_fn_uuid
    __wraped_cpp_fn_uuid += 1
    params = []
    cpp_args = []
    for arg_t, arg_name in zip(c_args_t, c_args_names):
        params.append(f"{arg_t.get_canonical().spelling} {arg_name}")
        cpp_type, c_to_cpp, _ = get_cpp_type(arg_t)
        if cpp_type != arg_t.spelling:
            cpp_args.append(c_to_cpp(arg_name))
        else:
            cpp_args.append(arg_name)

    ret_expr = f"to_call({','.join(cpp_args)})"
    cpp_type, _, cpp_to_c = get_cpp_type(c_ret_t)
    cpp_type_list = c_function_sig_to_cpp_function_sig(c_ret_t, c_args_t)
    cpp_type_list = [cpp_type_list[0]] + cpp_type_list[1]
    cpp_type_list_str = ','.join(cpp_type_list)

    if cpp_type != c_ret_t.spelling:
        ret_expr = cpp_to_c(ret_expr)

    c_wrapper = __cpp_function_to_c_template.format(
        params=','.join(params),
        cpp_args=','.join(cpp_args),
        cpp_type_list=cpp_type_list_str,
        uuid=__wraped_cpp_fn_uuid,
        ret_expr=ret_expr)

    fn_wrapper_t = f"pybind11_weaver::FnPointerWrapper<{cpp_type_list_str}>"
    c_fn_sig = f"{','.join([t.get_canonical().spelling for t in [c_ret_t] + c_args_t])}"
    return f"{fn_wrapper_t}::GetCptr<{c_fn_sig}>::Run({cpp_callable_name}, pybind11_weaver::Guardian() , {c_wrapper}, {__wraped_cpp_fn_uuid})"


def get_fn_value_expr(cursor: cindex.Cursor) -> str:
    cls_name = None
    if cursor.kind != cindex.CursorKind.CXCursor_FunctionDecl:
        cls_name = cursor.semantic_parent.spelling
    wrapper = wrap_c_function_to_cpp(cursor.spelling,
                                     cursor.result_type,
                                     [arg.type for arg in cursor.get_arguments()],
                                     [arg.spelling if arg.spelling != '' else f"arg{i}" for i, arg in
                                      enumerate(cursor.get_arguments())],
                                     cls_name)
    if wrapper:
        return wrapper
    else:
        pointer = f"&{scope_list.get_full_qualified_name(cursor)}"
        method_pointer_type = _get_fn_pointer_type(cursor)
        return f"static_cast<{method_pointer_type}>({pointer})"
