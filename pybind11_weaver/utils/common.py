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
        _logger.warning(f"Operator overloading not supported `{cursor.spelling}`")
    return is_operator


def is_concreate_template(cursor: cindex.Cursor) -> bool:
    # both template specialization and instantiation are concreate template
    return cursor.get_num_template_arguments() > 0


def _template_arg_name(cursor, idx, custom_mangle=False) -> Optional[str]:
    if cursor.get_template_argument_kind(idx) == cindex.TemplateArgumentKind.CXTemplateArgumentKind_Integral:
        return str(cursor.get_template_argument_value(idx))
    elif cursor.get_template_argument_kind(idx) == cindex.TemplateArgumentKind.CXTemplateArgumentKind_Type:
        t_name = cursor.get_template_argument_type(idx).spelling
        if custom_mangle:
            return t_name.replace(" ", "").replace("<", "_").replace(">", "_").replace(",", "_").replace("::", "_")
        else:
            return t_name
    raise NotImplementedError(f"template argument kind {cursor.get_template_argument_kind(idx)} not supported")


def get_safe_indentifier_name(cursor: cindex.Cursor) -> bool:
    if is_concreate_template(cursor):
        custom_mangle = cursor.spelling
        for i in range(cursor.get_num_template_arguments()):
            arg_name = _template_arg_name(cursor, i, custom_mangle=True)
            if arg_name is None:
                return cursor.mangled_name
            custom_mangle += "_" + arg_name
        return custom_mangle
    else:
        return cursor.spelling
