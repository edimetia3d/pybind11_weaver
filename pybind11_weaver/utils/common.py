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
