from typing import List, Tuple, Optional

from pylibclang import cindex

from . import scope_list

from pybind11_weaver.utils import common


def is_types_has_unique_ptr(types: List[cindex.Type]):
    for t in types:
        if "std::unique_ptr" in common.safe_type_reference(t):
            return True
    return False


def _get_fn_pointer_type(cursor: cindex.Cursor) -> Optional[str]:
    assert cursor.type.kind == cindex.TypeKind.CXType_FunctionProto
    """For libclang do not provide API to construct the pointer type, we have to construct it by ourself."""
    ret_t = common.safe_type_reference(cursor.result_type)
    args_t = [common.safe_type_reference(arg.type) for arg in cursor.get_arguments()]
    if cursor.kind == cindex.CursorKind.CXCursor_CXXMethod and not cursor.is_static_method():
        proto_spelling = cursor.type.spelling
        if proto_spelling.endswith("const &&"):
            return None
        elif proto_spelling.endswith("const &"):
            method_type = "const_lref_type"
        elif proto_spelling.endswith("&&"):
            return None
        elif proto_spelling.endswith("&"):
            method_type = "lref_type"
        elif proto_spelling.endswith("const"):
            method_type = "const_type"
        else:
            method_type = "type"

        cls_name = scope_list.get_full_qualified_name(cursor.semantic_parent)

        return f"pybind11_weaver::FnPtrT<{cls_name},{','.join([ret_t] + args_t)}>::{method_type}"

    else:
        return f"pybind11_weaver::FnPtrT<void,{ret_t}({','.join(args_t)})>::type"


_wrapped_db = set()


def get_wrapped_types():
    return _wrapped_db


class ValueCast:
    nest_level = -1

    def __init__(self, c_type: cindex.Type, cpp_type: str):
        self.c_type = c_type
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
        _wrapped_db.add(common.safe_type_reference(self.c_type))

    def cvt(self, c_expr: str, **kwargs):
        return f"pybind11_weaver::WrapP<{common.safe_type_reference(self.c_type)}>({c_expr})"


class WrappedToCPointer(ValueCast):
    def __init__(self, *args, **kwargs):
        ValueCast.__init__(self, *args, **kwargs)
        _wrapped_db.add(common.safe_type_reference(self.c_type))

    def cvt(self, cpp_expr: str, **kwargs):
        return f"({cpp_expr})->Cptr()"


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


def _get_cpp_type_from_proto(proto: cindex.TypeKind.CXType_FunctionProto):
    ret_t, args_t = c_function_sig_to_cpp_function_sig(proto.get_result(), proto.argument_types())
    cpp_type = f"std::function<{ret_t} ({','.join(args_t)})>"
    return cpp_type, CFnPointerToCppStdFn(proto, cpp_type), CppStdFnToCFnPointer(proto, cpp_type)


def get_cpp_type(c_type: cindex.Type) -> Tuple[str, "CValueToCppValue", "CppValueToCValue"]:
    c_type_spelling = common.safe_type_reference(c_type)
    ret = c_type_spelling, lambda x: x, lambda x: x

    canonical = c_type.get_canonical()
    if c_type_spelling.startswith("std::function"):
        _get_cpp_type_from_proto(
            canonical.get_template_argument_type(0))  # only make sure all types are insert to used_types
        return ret

    if canonical.kind == cindex.TypeKind.CXType_Pointer:
        pointee = canonical.get_pointee().get_canonical()
        if pointee.kind in [cindex.TypeKind.CXType_Pointer, cindex.TypeKind.CXType_Void]:
            cpp_type = f"pybind11_weaver::WrappedPtrT<{c_type_spelling}>"
            return cpp_type, CPointerToWrapped(canonical, cpp_type), WrappedToCPointer(canonical, cpp_type)
        if pointee.kind in [cindex.TypeKind.CXType_FunctionProto]:
            return _get_cpp_type_from_proto(pointee)
        # if pointee is a incompelete type, warp it
        pointee_decl = pointee.get_declaration()
        if pointee_decl.kind in [cindex.CursorKind.CXCursor_StructDecl,
                                 cindex.CursorKind.CXCursor_ClassDecl] and not pointee_decl.is_definition():
            cpp_type = f"pybind11_weaver::WrappedPtrT<{c_type_spelling}>"
            return cpp_type, CPointerToWrapped(canonical, cpp_type), WrappedToCPointer(canonical, cpp_type)
        common.add_used_types(pointee)
    else:
        common.add_used_types(canonical)

    return ret


__c_function_to_cpp_template = """[]({params}){{
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
        cpp_type, _, cpp_to_c = get_cpp_type(param_t)
        if cpp_type != common.safe_type_reference(param_t):
            wrap_param = True
            new_params.append(f"{cpp_type} {param_spelling}")
            forward_args.append(cpp_to_c(param_spelling))
        else:
            new_params.append(f"{common.safe_type_reference(param_t)} {param_spelling}")
            forward_args.append(param_spelling)
    ret_expr = f"{fn_name}({','.join(forward_args)})"
    if cls_name:
        ret_expr = f"self.{ret_expr}"

    # ret
    cpp_type, c_to_cpp, _ = get_cpp_type(ret_t)
    wrap_ret = False
    if cpp_type != common.safe_type_reference(ret_t):
        wrap_ret = True
        ret_expr = c_to_cpp(ret_expr)
    if wrap_param or wrap_ret or force_warp:
        return __c_function_to_cpp_template.format(params=','.join(new_params), ret_expr=ret_expr)
    else:
        return None


__cpp_function_to_c_template = """[]({params}){{
    auto to_call = pybind11_weaver::FnPointerWrapper<{cpp_type_list}>::GetFnProxy(__DATE__ __TIME__ __FILE__ , __COUNTER__);
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
    params = []
    cpp_args = []
    for arg_t, arg_name in zip(c_args_t, c_args_names):
        params.append(f"{common.safe_type_reference(arg_t)} {arg_name}")
        cpp_type, c_to_cpp, _ = get_cpp_type(arg_t)
        if cpp_type != common.safe_type_reference(arg_t):
            cpp_args.append(c_to_cpp(arg_name))
        else:
            cpp_args.append(arg_name)

    ret_expr = f"to_call({','.join(cpp_args)})"
    cpp_type, _, cpp_to_c = get_cpp_type(c_ret_t)
    cpp_type_list = c_function_sig_to_cpp_function_sig(c_ret_t, c_args_t)
    cpp_type_list = [cpp_type_list[0]] + cpp_type_list[1]
    cpp_type_list_str = ','.join(cpp_type_list)

    if cpp_type != common.safe_type_reference(c_ret_t):
        ret_expr = cpp_to_c(ret_expr)

    c_wrapper = __cpp_function_to_c_template.format(
        params=','.join(params),
        cpp_args=','.join(cpp_args),
        cpp_type_list=cpp_type_list_str,
        ret_expr=ret_expr)

    fn_wrapper_t = f"pybind11_weaver::FnPointerWrapper<{cpp_type_list_str}>"
    c_fn_sig = f"{','.join([common.safe_type_reference(t) for t in [c_ret_t] + c_args_t])}"
    return f"""{fn_wrapper_t}::GetCptr<{c_fn_sig}>::Run({cpp_callable_name}, pybind11_weaver::Guardian() , {c_wrapper}, 
/* clang-format off */
__DATE__ __TIME__ __FILE__, 
__COUNTER__ - 1
/* clang-format on */
)"""


def _fn_template_arg_name(cursor, idx, as_python_name=False) -> Optional[str]:
    if cursor.get_template_argument_kind(idx) == cindex.TemplateArgumentKind.CXTemplateArgumentKind_Integral:
        return str(cursor.get_template_argument_value(idx))
    elif cursor.get_template_argument_kind(idx) == cindex.TemplateArgumentKind.CXTemplateArgumentKind_Type:
        if as_python_name:
            return common.type_python_name(cursor.get_template_argument_type(idx).spelling)
        else:
            return common.safe_type_reference(cursor.get_template_argument_type(idx))
    raise NotImplementedError(f"template argument kind {cursor.get_template_argument_kind(idx)} not supported")


def fn_python_name(cursor: cindex.Cursor) -> str:
    if common.is_concreate_template(cursor):
        custom_mangle = cursor.spelling
        for i in range(cursor.get_num_template_arguments()):
            arg_name = _fn_template_arg_name(cursor, i, as_python_name=True)
            if arg_name is None:
                return cursor.mangled_name
            custom_mangle += "_" + arg_name
        return custom_mangle
    else:
        return cursor.spelling


def fn_ref_name(cursor: cindex.Cursor) -> Optional[str]:
    args = []
    if common.is_concreate_template(cursor):
        for i in range(cursor.get_num_template_arguments()):
            arg_name = _fn_template_arg_name(cursor, i, as_python_name=False)
            args.append(arg_name)
        base_name = f"{cursor.spelling}<{','.join(args)}>"
    else:
        base_name = cursor.spelling
    return scope_list.get_full_qualified_name(cursor, base_name=base_name)


def get_fn_value_expr(cursor: cindex.Cursor) -> Optional[str]:
    if is_types_has_unique_ptr([arg.type for arg in cursor.get_arguments()] + [cursor.result_type]):
        return None
    cls_name = None
    if cursor.kind != cindex.CursorKind.CXCursor_FunctionDecl:
        cursor.semantic_parent._tu = cursor._tu
        cls_name = common.safe_type_reference(cursor.semantic_parent.type)
    ref_name = fn_ref_name(cursor)
    wrapper = wrap_c_function_to_cpp(ref_name,
                                     cursor.result_type,
                                     [arg.type for arg in cursor.get_arguments()],
                                     [arg.spelling if arg.spelling != '' else f"arg{i}" for i, arg in
                                      enumerate(cursor.get_arguments())],
                                     cls_name)
    if wrapper:
        return wrapper
    else:
        fn_t = _get_fn_pointer_type(cursor)
        if fn_t is None:
            return None
        else:
            return f"static_cast<{_get_fn_pointer_type(cursor)}>(&{ref_name})"
