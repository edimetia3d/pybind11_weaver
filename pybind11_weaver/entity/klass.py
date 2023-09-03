from typing import List

from clang import cindex

from . import entity_base
from pybind11_weaver.utils import fn

import logging

_logger = logging.getLogger(__name__)


class ClassEntity(entity_base.Entity):

    def __init__(self, cursor: cindex.Cursor):
        entity_base.Entity.__init__(self, cursor)
        assert cursor.kind in [cindex.CursorKind.CLASS_DECL, cindex.CursorKind.STRUCT_DECL]

    def get_cpp_struct_name(self) -> str:
        return self.cursor.type.spelling.replace("::", "_")

    def create_pybind11_obj_expr(self, module_sym: str) -> str:
        code = f'{self.pybind11_type_str()}({module_sym},"{self.name}")'
        return code

    def update_stmts(self, pybind11_obj_sym: str) -> List[str]:
        codes = []

        def is_pubic(cursor):
            return cursor.access_specifier == cindex.AccessSpecifier.PUBLIC

        def is_virtual(cursor):
            return cursor.is_virtual_method() or cursor.is_pure_virtual_method()

        def is_static(cursor):
            return cursor.is_static_method()

        # generate constructor binding
        ctor_found = False
        for cursor in self.cursor.get_children():
            if cursor.kind == cindex.CursorKind.CONSTRUCTOR and is_pubic(cursor):
                ctor_found = True
                param_types = fn.fn_arg_type(cursor)
                codes.append(
                    f"{pybind11_obj_sym}.def(pybind11::init<{','.join(param_types)}>());")
        if not ctor_found:
            codes.append(f"{pybind11_obj_sym}.def(pybind11::init<>());")

        # generate method binding
        for cursor in self.cursor.get_children():
            if cursor.kind == cindex.CursorKind.CXX_METHOD and is_pubic(cursor):
                # to support overload, we will cast member function to function pointer
                pointer = f"&{self.qualified_name()}::{cursor.spelling}"
                method_pointer_type = fn.get_fn_pointer_type(cursor)
                casted_pointer = f"static_cast<{method_pointer_type}>({pointer})"
                if is_static(cursor):
                    codes.append(
                        f"{pybind11_obj_sym}.def_static(\"{cursor.spelling}\",{casted_pointer});")
                else:
                    codes.append(
                        f"{pybind11_obj_sym}.def(\"{cursor.spelling}\",{casted_pointer});")
                    if is_virtual(cursor):
                        _logger.warning(
                            f"virtual method {cursor.spelling} is not fully supported yet, override in python is not allowed.")

        # generate field binding
        for cursor in self.cursor.get_children():
            if cursor.kind == cindex.CursorKind.FIELD_DECL and is_pubic(cursor):
                codes.append(
                    f"{pybind11_obj_sym}.def_readwrite(\"{cursor.spelling}\",&{self.qualified_name()}::{cursor.spelling});")

        return codes

    def pybind11_type_str(self) -> str:
        type_full_name = self.cursor.type.spelling
        return f"pybind11::class_<{type_full_name}>"
