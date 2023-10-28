import logging

from typing import List, Tuple

from pylibclang import cindex

from pybind11_weaver.utils import common, fn
from pybind11_weaver.entity import entity_base
from . import klass

_logger = logging.getLogger(__name__)


def _is_bindable(cursor: cindex.Cursor):
    c_type = common.remove_const_ref_pointer(cursor.type)
    canonical = c_type.get_canonical()
    if int(canonical.kind) <= int(cindex.TypeKind.CXType_LastBuiltin) and int(canonical.kind) >= int(
            cindex.TypeKind.CXType_FirstBuiltin):
        return True
    if canonical.kind == cindex.TypeKind.CXType_Enum:
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
                    kls_entity.could_member_export(cursor):
                if common.is_types_has_unique_ptr([cursor.type]) or \
                        not isinstance(fn.get_pb11_type(cursor.type)[1], fn.NoCast) or \
                        not _is_bindable(cursor):
                    _logger.info(f"Can not gen field {cursor.spelling} of {kls_entity.reference_name()}")
                    continue
                filed_binder = "def_readwrite"
                if cursor.type.is_const_qualified():
                    filed_binder = "def_readonly"
                codes.append(
                    f"{pybind11_obj_sym}.{filed_binder}(\"{cursor.spelling}\",&{kls_entity.reference_name()}::{cursor.spelling});")
                if kls_entity.gu.io_config.gen_docstring:
                    codes[-1] = entity_base._inject_docstring(codes[-1], cursor, "last_arg")
        return codes, []
