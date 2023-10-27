import logging

from typing import List, Tuple

from pylibclang import cindex

from pybind11_weaver.utils import fn
from pybind11_weaver.entity import entity_base
from . import klass

_logger = logging.getLogger(__name__)


def _is_bindable_type(type: cindex.Type):
    canonical = type.get_canonical()
    if canonical.kind < cindex.TypeKind.CXType_LastBuiltin and canonical.kind > cindex.TypeKind.CXType_FirstBuiltin:
        return True
    if canonical.spelling in ["std::string", "char*", "const char*"]:
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
                    kls_entity.could_export(cursor) and \
                    _is_bindable_type(cursor.type):
                filed_binder = "def_readwrite"
                if cursor.type.is_const_qualified():
                    filed_binder = "def_readonly"
                codes.append(
                    f"{pybind11_obj_sym}.{filed_binder}(\"{cursor.spelling}\",&{kls_entity.reference_name()}::{cursor.spelling});")
                if kls_entity.gu.io_config.gen_docstring:
                    codes[-1] = entity_base._inject_docstring(codes[-1], cursor, "last_arg")
        return codes, []
