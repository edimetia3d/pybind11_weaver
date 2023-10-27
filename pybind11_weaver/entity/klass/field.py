import logging

from typing import List, Tuple

from pylibclang import cindex

from pybind11_weaver.utils import common
from pybind11_weaver.entity import entity_base
from . import klass

_logger = logging.getLogger(__name__)


def _is_writable(cursor: cindex.Cursor):
    if cursor.type.is_const_qualified():
        return False
    canonical = cursor.type.get_canonical()
    if canonical.kind < cindex.TypeKind.CXType_LastBuiltin and canonical.kind > cindex.TypeKind.CXType_FirstBuiltin:
        return True
    if canonical.spelling in ["std::string", "std::basic_string<char>"]:
        return True
    return False


class GenFiled:

    def __init__(self, kls_entity: "klass.KlassEntity"):
        self.kls_entity = kls_entity

    def run(self, pybind11_obj_sym: str) -> Tuple[List[str], List[str]]:
        codes = []
        kls_entity = self.kls_entity
        for cursor in kls_entity.cursor.get_children():
            if cursor.kind == cindex.CursorKind.CXCursor_FieldDecl and \
                    kls_entity.could_export(cursor) and not common.is_types_has_unique_ptr([cursor.type]):
                if _is_writable(cursor):
                    filed_binder = "def_readwrite"
                else:
                    filed_binder = "def_readonly"
                codes.append(
                    f"{pybind11_obj_sym}.{filed_binder}(\"{cursor.spelling}\",&{kls_entity.reference_name()}::{cursor.spelling});")
                common.add_used_types(cursor.type)
                if kls_entity.gu.io_config.gen_docstring:
                    codes[-1] = entity_base._inject_docstring(codes[-1], cursor, "last_arg")
        return codes, []
