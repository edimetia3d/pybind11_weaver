from typing import List, Tuple, Optional, Union

from pylibclang import cindex

from . import scope_list

from pybind11_weaver.utils import common


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

    def __init__(self, c_type: cindex.Type, pb11_type: str):
        self.c_type = c_type
        self.pb11_type = pb11_type

    def __call__(self, value: str, **kwargs):
        self.nest_level += 1
        ret = self.cvt(value, **kwargs)
        self.nest_level -= 1
        return ret

    def cvt(self, value: str, **kwargs):
        pass


class NoCast(ValueCast):
    def cvt(self, value: str, **kwargs):
        return value


class PointerToPb11Value(ValueCast):

    def __init__(self, *args, **kwargs):
        ValueCast.__init__(self, *args, **kwargs)
        _wrapped_db.add(self.c_type)

    def cvt(self, c_value: str, **kwargs):
        return f"pybind11_weaver::WrapP<{common.safe_type_reference(self.c_type)}>({c_value})"


class Pb11ValueToPointer(ValueCast):
    def __init__(self, *args, **kwargs):
        ValueCast.__init__(self, *args, **kwargs)
        _wrapped_db.add(self.c_type)

    def cvt(self, pb11_value: str, **kwargs):
        return f"({pb11_value})->Cptr()"


class FnPointerToPb11Fn(ValueCast):

    def cvt(self, callable_with_c_type: str, **kwargsk):
        if "capture_list" in kwargsk:
            capture_list = kwargsk["capture_list"]
        else:
            capture_list = ""
        assert self.c_type.kind == cindex.TypeKind.CXType_FunctionProto
        return wrap_c_callable_in_pb11_type_io(f"({callable_with_c_type})",
                                               self.c_type.get_result(),
                                               [d for d in self.c_type.argument_types()],
                                               [f"arg{self.nest_level}_{i}" for i, _ in
                                                enumerate(self.c_type.argument_types())],
                                               capture_list,
                                               force_wrap=True)


class Pb11FnToFnPointer(ValueCast):

    def cvt(self, callable_with_pb11_type: str, **kwargs):
        assert self.c_type.kind == cindex.TypeKind.CXType_FunctionProto
        return wrap_pb11_fn_in_c_type_io(f"({callable_with_pb11_type})", self.c_type.get_result(),
                                         [d for d in self.c_type.argument_types()],
                                         [f"arg{self.nest_level}_{i}" for i, _ in
                                          enumerate(self.c_type.argument_types())])


def get_pb11_type(c_type: cindex.Type) -> Tuple[str, "CValuePb11Value", "Pb11ValueToCValue"]:
    c_type_spelling = common.safe_type_reference(c_type)
    canonical = c_type.get_canonical()
    ret = c_type_spelling, NoCast(canonical, c_type_spelling), NoCast(canonical, c_type_spelling)

    def to_pb11_fn(proto: cindex.TypeKind.CXType_FunctionProto):
        ret_t, args_t, casted = _c_io_type_to_pb11_io_type(proto.get_result(), proto.argument_types())
        if casted:
            pb11_type = f"std::function<{ret_t} ({','.join(args_t)})>"
            return pb11_type, FnPointerToPb11Fn(proto, pb11_type), Pb11FnToFnPointer(proto, pb11_type)
        else:
            fn_ptr_t = f"{ret_t} (*)({','.join(args_t)})"
            return fn_ptr_t, NoCast(proto, fn_ptr_t), NoCast(proto, fn_ptr_t)

    def to_warped_ptr():
        pb11_type = f"pybind11_weaver::WrappedPtrT<{c_type_spelling}>"
        return pb11_type, PointerToPb11Value(canonical, pb11_type), Pb11ValueToPointer(canonical, pb11_type)

    if c_type_spelling.startswith("std::function"):
        return to_pb11_fn(canonical.get_template_argument_type(0))

    if canonical.kind == cindex.TypeKind.CXType_Pointer:
        pointee = canonical.get_pointee().get_canonical()
        if pointee.kind in [cindex.TypeKind.CXType_Pointer, cindex.TypeKind.CXType_Void]:
            return to_warped_ptr()
        if pointee.kind in [cindex.TypeKind.CXType_FunctionProto]:
            return to_pb11_fn(pointee)
        # if pointee is a incompelete type, wrap it
        pointee_decl = pointee.get_declaration()
        if pointee_decl.kind in [cindex.CursorKind.CXCursor_StructDecl,
                                 cindex.CursorKind.CXCursor_ClassDecl] and not pointee_decl.is_definition():
            return to_warped_ptr()
        if not common.is_type_deletable(pointee):
            return to_warped_ptr()
        common.add_used_types(pointee)
    else:
        common.add_used_types(canonical)

    return ret


__c_callable_inside = """[{capture_list}]({params}){{
    {ret_expr};
}}"""


def wrap_c_callable_in_pb11_type_io(c_callable_name: str, ret_t: cindex.Type, args_t: List[cindex.Type],
                                    args_names: List[str],
                                    capture_list: Union[str, None],
                                    cls_name: str = None, force_wrap=False, is_static=False):
    if capture_list is None:
        capture_list = ""
    # param and forwarded args
    new_params = []
    if cls_name:
        new_params.append(f"{cls_name}& self")
    forward_args = []
    wrap_param = False
    for param_t, param_spelling in zip(args_t, args_names):
        pb11_type, _, pb11_value_to_c = get_pb11_type(param_t)
        if isinstance(pb11_value_to_c, NoCast):
            new_params.append(f"{common.safe_type_reference(param_t)} {param_spelling}")
            forward_args.append(param_spelling)
        else:
            wrap_param = True
            new_params.append(f"{pb11_type} {param_spelling}")
            forward_args.append(pb11_value_to_c(param_spelling))

    if cls_name and not is_static:
        forward_args = [c_callable_name, "&self"] + forward_args
    else:
        forward_args = [c_callable_name] + forward_args
    ret_expr = f"std::invoke({','.join(forward_args)})"

    # ret
    _, c_value_to_pb11, _ = get_pb11_type(ret_t)
    wrap_ret = False
    ret_t_is_void = ret_t.kind == cindex.TypeKind.CXType_Void
    if not ret_t_is_void and not isinstance(c_value_to_pb11, NoCast):
        wrap_ret = True
        ret_expr = f"auto && __ret__ = {ret_expr} ; return {c_value_to_pb11('__ret__', capture_list='__ret__=std::move(__ret__)')}"
    else:
        ret_expr = f"return {ret_expr}"
    if wrap_param or wrap_ret or force_wrap:
        return __c_callable_inside.format(params=','.join(new_params), ret_expr=ret_expr, capture_list=capture_list)
    else:
        return None


__pb11_callable_inside = """[]({params}){{
    auto to_call = pybind11_weaver::FnPointerWrapper<{pb11_io_type}>::GetFnProxy(__DATE__ __TIME__ __FILE__ , __COUNTER__);
    {ret_expr};
}}"""


def _c_io_type_to_pb11_io_type(c_ret_t, c_args_t) -> Tuple[str, List[str], bool]:
    ret_pb11_t, c_to_pb11, _ = get_pb11_type(c_ret_t)
    py11_args_t = []
    casted = not isinstance(c_to_pb11, NoCast)
    for a in c_args_t:
        pb11_t, c_to_pb11, _ = get_pb11_type(a)
        py11_args_t.append(pb11_t)
        casted = casted or not isinstance(c_to_pb11, NoCast)

    return ret_pb11_t, py11_args_t, casted


def wrap_pb11_fn_in_c_type_io(pb11_callable_name: str,
                              c_ret_t: cindex.Type,
                              c_args_t: List[cindex.Type],
                              c_args_names: List[str]):
    params = []
    forward_args = []
    for arg_t, arg_name in zip(c_args_t, c_args_names):
        params.append(f"{common.safe_type_reference(arg_t)} {arg_name}")
        pb11_type, c_value_to_pb11, _ = get_pb11_type(arg_t)
        if isinstance(c_value_to_pb11, NoCast):
            forward_args.append(arg_name)
        else:
            forward_args.append(c_value_to_pb11(arg_name, capture_list=f"{arg_name}"))

    ret_expr = f"to_call({','.join(forward_args)})"
    pb11_ret_type, _, pb11_value_to_c = get_pb11_type(c_ret_t)
    pb11_io_t = _c_io_type_to_pb11_io_type(c_ret_t, c_args_t)
    pb11_io_t = [pb11_io_t[0]] + pb11_io_t[1]
    pb11_io_t_str = ','.join(pb11_io_t)

    ret_t_is_void = c_ret_t.kind == cindex.TypeKind.CXType_Void
    if not ret_t_is_void and not isinstance(pb11_value_to_c, NoCast):
        ret_expr = f"auto && __pb11_ret__= ret_expr; return {pb11_value_to_c('__pb11_ret__')}"
    else:
        ret_expr = f"return {ret_expr}"

    c_wrapper = __pb11_callable_inside.format(
        params=','.join(params),
        pb11_io_type=pb11_io_t_str,
        ret_expr=ret_expr)

    fn_wrapper_t = f"pybind11_weaver::FnPointerWrapper<{pb11_io_t_str}>"
    c_fn_sig = f"{','.join([common.safe_type_reference(t) for t in [c_ret_t] + c_args_t])}"
    return f"""{fn_wrapper_t}::GetCptr<{c_fn_sig}>::Run({pb11_callable_name}, pybind11_weaver::Guardian() , {c_wrapper}, 
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
    if common.is_types_has_unique_ptr([arg.type for arg in cursor.get_arguments()] + [cursor.result_type]):
        return None
    fn_t = _get_fn_pointer_type(cursor)
    if fn_t is None:
        return None
    cls_name = None
    if cursor.kind != cindex.CursorKind.CXCursor_FunctionDecl:
        cursor.semantic_parent._tu = cursor._tu
        cls_name = common.safe_type_reference(cursor.semantic_parent.type)
    is_static = cursor.kind == cindex.CursorKind.CXCursor_CXXMethod and cursor.is_static_method()

    ref_name = fn_ref_name(cursor)
    fn_ptr = f"static_cast<{fn_t}>(&{ref_name})"
    wrapper = wrap_c_callable_in_pb11_type_io(fn_ptr,
                                              cursor.result_type,
                                              [arg.type for arg in cursor.get_arguments()],
                                              [arg.spelling if arg.spelling != '' else f"arg{i}" for i, arg in
                                               enumerate(cursor.get_arguments())],
                                              None,
                                              cls_name, is_static=is_static)
    if wrapper:
        return wrapper
    else:
        return fn_ptr
