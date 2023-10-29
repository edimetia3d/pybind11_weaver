from typing import List, Tuple, Optional, Dict

from pylibclang import cindex
import pylibclang._C
import logging

_logger = logging.getLogger(__name__)


def is_public(cursor: cindex.Cursor) -> bool:
    return cursor.access_specifier not in [cindex.AccessSpecifier.CX_CXXPrivate,
                                           cindex.AccessSpecifier.CX_CXXProtected]


def is_visible(cursor: cindex.Cursor, strcit_mode: bool) -> bool:
    vis = pylibclang._C.clang_getCursorVisibility(cursor) == cindex._C.CXVisibilityKind.CXVisibility_Default
    lazy_inline_check = cursor.is_definition() and ".h" in str(cursor.location.file)
    strict_inline_check = pylibclang._C.clang_Cursor_isFunctionInlined(
        cursor) or pylibclang._C.clang_Cursor_isInlineNamespace(cursor)
    vis = vis or lazy_inline_check or strict_inline_check
    if strcit_mode:
        return vis
    else:
        should_check = cursor.kind in [cindex.CursorKind.CXCursor_Constructor, cindex.CursorKind.CXCursor_CXXMethod,
                                       cindex.CursorKind.CXCursor_FunctionDecl]
        return vis if should_check else True


def is_operator_overload(cursor: cindex.Cursor) -> bool:
    is_operator = "operator" in cursor.spelling
    if is_operator:
        _logger.info(
            f"Operator overloading not supported `{cursor.spelling}` at {cursor.location.file}:{cursor.location.line}")
    return is_operator


def is_concreate_template(cursor: cindex.Cursor) -> bool:
    # both template specialization and instantiation are concreate template
    return cursor.get_num_template_arguments() > 0


_type_spliter = {":", " ", ",", "<", ">", "&", "*", "(", ")", "&"}

# map type spliter into valid identifier character
_type_spliter_map = {"<": "6", ">": "9", "::": "_", "&&": "_", "(": "6", "(": "9", "*": "p", " ": ""}


def type_python_name(name) -> str:
    """When a type's fullname contains < or > , must be mangled to be able to use in python"""
    for k, v in _type_spliter_map.items():
        name = name.replace(k, v)
    for k in _type_spliter:
        name = name.replace(k, "_")
    return name


_used_types = set()


def get_used_types():
    """All used types will be ensured to be exposed to python, even if they are not binded directly"""
    return _used_types


def _sub_type_str(type_str, subs: Dict[str, str]):
    # parse type_str manually, to get all identifiers and keywords

    tokens = []
    i = 0
    end = len(type_str)
    while i < end:
        if type_str[i] in [":", "&"]:
            tag = type_str[i]
            value = tag + tag
            if type_str[i + 1] == tag:
                tokens.append(value)
                i += 2
                continue
            else:
                tokens.append(tag)
                i += 1
                continue
        elif type_str[i] in _type_spliter:
            tokens.append(type_str[i])
            i += 1
            continue
        else:
            start = i
            while i < end and type_str[i] not in _type_spliter:
                i += 1
            new_token = type_str[start:i]
            if new_token in subs:
                tokens.append(subs[new_token])
            elif new_token != "typename":
                tokens.append(new_token)
            continue
    ret = "".join(tokens).replace(" ", "")
    return ret


def safe_type_reference(type: cindex.Type, subs: Dict[str, str] = None) -> str:
    ret = type.get_canonical().spelling
    if "type-parameter" in ret:
        if subs is None:
            return type.spelling  # type is a template parameter
        else:
            return _sub_type_str(type.spelling, subs)
    return ret


def add_used_types(type: cindex.Type):
    canonical = type.get_canonical()
    if int(canonical.kind) > int(
            cindex.TypeKind.CXType_LastBuiltin) and "std::" not in canonical.get_canonical().spelling:
        _used_types.add(safe_type_reference(remove_const_ref_pointer(type)))


def remove_const_ref(type: cindex.Type):
    tu = type._tu
    type = pylibclang._C.clang_getUnqualifiedType(pylibclang._C.clang_getNonReferenceType(type))
    type._tu = tu
    return type


def remove_pointer(type: cindex.Type):
    if type.kind in [cindex.TypeKind.CXType_Pointer]:
        type = type.get_pointee()
    return type


def remove_const_ref_pointer(type: cindex.Type):
    tu = type._tu
    type = remove_const_ref(type)
    type._tu = tu
    if type.kind in [cindex.TypeKind.CXType_Pointer]:
        return remove_const_ref_pointer(type.get_pointee())
    else:
        return type


def is_types_has_unique_ptr(types: List[cindex.Type]):
    for t in types:
        if "std::unique_ptr" in safe_type_reference(t):
            return True
    return False


def could_member_accessed(cursor: cindex.Cursor):
    return is_public(cursor) and not cursor.is_deleted_method() and is_visible(
        cursor, True)


def is_marked_final(cursor: cindex.Cursor):
    for c in cursor.get_children():
        if c.kind == cindex.CursorKind.CXCursor_CXXFinalAttr:
            return True
    return False


_deletable_db = dict()


def is_type_deletable(type: cindex.Type):
    cursor = type.get_declaration()
    deletable = True
    if cursor.kind not in [cindex.CursorKind.CXCursor_ClassDecl, cindex.CursorKind.CXCursor_StructDecl]:
        return True
    cursor, _, _ = get_def_cls_cursor(cursor)

    for c in cursor.get_children():
        if c.kind == cindex.CursorKind.CXCursor_Destructor and not could_member_accessed(c):
            deletable = False
        if c.kind == cindex.CursorKind.CXCursor_CXXMethod and c.spelling == "operator delete":
            if not could_member_accessed(c):
                deletable = False
        if c.kind == cindex.CursorKind.CXCursor_CXXBaseSpecifier:
            base_name = safe_type_reference(c.type)
            if base_name in _deletable_db:
                deletable = _deletable_db[base_name]
            else:
                if not is_type_deletable(c.type):
                    deletable = False
        if not deletable:
            break
    _deletable_db[safe_type_reference(type)] = deletable
    return deletable


def _is_explicit_instantiation(cursor: cindex.Cursor):
    tokens = [x.spelling for x in cursor.get_tokens()]
    for i, token in enumerate(tokens):
        if token == "template":
            if tokens[i + 1] == "<":
                return False
            else:
                if tokens[i + 1] == "class" or tokens[i + 1] == "struct":
                    return True
                else:
                    raise NotImplementedError("Only class and struct explicit instantiation supported for now")


def _get_template_param_arg_pair(template_cursor: cindex.Cursor, specialized_cursor: cindex.Cursor) -> Optional[Tuple[
    List[str], Dict[str, str]]]:
    decls = []
    subs = dict()
    for cursor in template_cursor.get_children():
        if cursor.kind == cindex.CursorKind.CXCursor_TemplateTypeParameter:
            assert specialized_cursor.get_template_argument_kind(
                len(decls)) == cindex.TemplateArgumentKind.CXTemplateArgumentKind_Type
            t_param = cursor.spelling
            substitute = safe_type_reference(specialized_cursor.get_template_argument_type(len(decls)))
            decls.append(f"using {t_param} = {substitute};")
            subs[t_param] = substitute
        elif cursor.kind == cindex.CursorKind.CXCursor_NonTypeTemplateParameter:
            if specialized_cursor.get_template_argument_kind(
                    len(decls)) == cindex.TemplateArgumentKind.CXTemplateArgumentKind_Integral:
                t_param = cursor.spelling
                substitute = specialized_cursor.get_template_argument_value(len(decls))
                decls.append(
                    f"static constexpr int {t_param} = {substitute};")
                subs[t_param] = substitute
            else:
                _logger.error("Only Type and int template parameter supported for now")
                return None, None
    return decls, subs


def get_def_cls_cursor(cursor: cindex.Cursor) -> Tuple[cindex.Cursor, Optional[List[str]], Optional[Dict[str, str]]]:
    assert cursor.kind in [cindex.CursorKind.CXCursor_ClassDecl, cindex.CursorKind.CXCursor_StructDecl]
    template_decls = []
    subs = dict()
    ret_cursor = cursor
    if is_concreate_template(cursor):
        template_cursor = pylibclang._C.clang_getSpecializedCursorTemplate(cursor)
        template_cursor._tu = cursor._tu
        template_decls, subs = _get_template_param_arg_pair(template_cursor, cursor)
        if _is_explicit_instantiation(cursor):
            ret_cursor = template_cursor  # Only when instantiation, return the template cursor

    return ret_cursor, template_decls, subs
