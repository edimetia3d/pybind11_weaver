import logging

from . import entity_base
from . import enum
from . import klass
from . import namespace
from . import funktion
from pybind11_weaver.utils import common

from pylibclang import cindex

from pybind11_weaver import gen_unit

_logger = logging.getLogger(__name__)
_KIND = cindex.CursorKind


def create_entity(gu: gen_unit.GenUnit, cursor: cindex.Cursor):
    """Create an entity without parent.

    Note:
         newly created entity has no parent, user must call update_parent() after creation.

         update_parent() will update the parent of the entity and add the entity to the parent's children.

    """
    kind = cursor.kind

    if kind == _KIND.CXCursor_EnumDecl and cursor.is_definition():
        return enum.EnumEntity(gu, cursor)
    if kind == _KIND.CXCursor_Namespace:
        return namespace.NamespaceEntity(gu, cursor)
    if kind in [_KIND.CXCursor_ClassDecl,
                _KIND.CXCursor_StructDecl] and cursor.is_definition():
        return klass.ClassEntity(gu, cursor)
    if kind == _KIND.CXCursor_FunctionDecl:
        if common.is_operator_overload(cursor):
            pass  # handle later
        else:
            return funktion.FunctionEntity(gu, cursor)

    return None
