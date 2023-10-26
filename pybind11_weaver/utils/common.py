from typing import Optional

from pylibclang import cindex
import pylibclang._C
import logging

_logger = logging.getLogger(__name__)


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


def type_python_name(name) -> str:
    """When a type's fullname contains < or > , must be mangled to be able to use in python"""
    return name.replace("<", "6").replace(">", "9").replace(" ", "").replace(",", "_").replace("::", "_")


_used_types = set()


def get_used_types():
    """All used types will be ensured to be exposed to python, even if they are not binded directly"""
    return _used_types


def safe_type_reference(type: cindex.Type):
    ret = type.get_canonical().spelling
    if "type-parameter" in ret:
        return type.spelling  # type is a template parameter
    return ret


def add_used_types(type: cindex.Type):
    canonical = type.get_canonical()
    if int(canonical.kind) > int(
            cindex.TypeKind.CXType_LastBuiltin) and "std::" not in canonical.get_canonical().spelling:
        _used_types.add(safe_type_reference(type))


def remove_const_ref_pointer(type: cindex.Type):
    tu = type._tu
    type = pylibclang._C.clang_getUnqualifiedType(pylibclang._C.clang_getNonReferenceType(type))
    type._tu = tu
    if type.kind in [cindex.TypeKind.CXType_Pointer]:
        return remove_const_ref_pointer(type.get_pointee())
    else:
        return type
