from typing import List, Dict

from clang import cindex

from . import entity_base
from pybind11_weaver.utils import fn

import logging

_logger = logging.getLogger(__name__)


class MethodCoder:

    def __init__(self, cursor, scope_full_name):
        self.fn_name = cursor.spelling
        self.call_stmt = f"""BindMethod_{self.fn_name}(obj);"""
        self.def_template = f"""
virtual void BindMethod_{self.fn_name}(Pybind11T & obj){{{{
    {{body}}
}}}}
"""
        self.body = []
        self.append(cursor, scope_full_name)

    def get_def_stmt(self):
        return self.def_template.format(body="\n".join(self.body))

    def append(self, cursor, scope_full_name):
        def is_virtual(cursor):
            return cursor.is_virtual_method() or cursor.is_pure_virtual_method()

        def is_static(cursor):
            return cursor.is_static_method()

        # to support overload, we will cast member function to function pointer
        pointer = f"&{scope_full_name}::{cursor.spelling}"
        method_pointer_type = fn.get_fn_pointer_type(cursor)
        casted_pointer = f"static_cast<{method_pointer_type}>({pointer})"
        if is_static(cursor):
            self.body.append(
                f"obj.def_static(\"{cursor.spelling}\",{casted_pointer});")
        else:
            self.body.append(
                f"obj.def(\"{cursor.spelling}\",{casted_pointer});")
            if is_virtual(cursor):
                _logger.warning(
                    f"virtual method {cursor.spelling} is not fully supported yet, override in python is not allowed.")


class ClassEntity(entity_base.Entity):

    def __init__(self, cursor: cindex.Cursor):
        entity_base.Entity.__init__(self, cursor)
        assert cursor.kind in [cindex.CursorKind.CLASS_DECL, cindex.CursorKind.STRUCT_DECL]
        self.extra_methods_codes = []
        self.is_visible_fn = None  # set by entity_tree

    def get_cpp_struct_name(self) -> str:
        return self.cursor.type.spelling.replace("::", "_")

    def init_default_pybind11_value(self, parent_scope_sym: str) -> str:
        code = f'{parent_scope_sym},"{self.name}"'
        return code

    def update_stmts(self, pybind11_obj_sym: str) -> List[str]:
        codes = []

        def is_pubic(cursor):
            return cursor.access_specifier == cindex.AccessSpecifier.PUBLIC and self.is_visible_fn(cursor)

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
        methods: Dict[str, MethodCoder] = dict()
        for cursor in self.cursor.get_children():
            if cursor.kind == cindex.CursorKind.CXX_METHOD and is_pubic(cursor):
                if not cursor.spelling in methods:
                    methods[cursor.spelling] = MethodCoder(cursor, self.qualified_name())
                else:
                    methods[cursor.spelling].append(cursor, self.qualified_name())

        codes.append(f"Pybind11WeaverBindAllMethods({pybind11_obj_sym});")
        call_method_bind = []
        method_bind_body = []
        for _, method in methods.items():
            call_method_bind.append(method.call_stmt)
            method_bind_body.append(method.get_def_stmt())
        new_line = "\n"
        bind_all = f"""
void Pybind11WeaverBindAllMethods(Pybind11T & obj){{
   {new_line.join(call_method_bind)}
}}

{new_line.join(method_bind_body)}
        """
        self.extra_methods_codes.append(bind_all)

        # generate field binding
        for cursor in self.cursor.get_children():
            if cursor.kind == cindex.CursorKind.FIELD_DECL and is_pubic(cursor):
                codes.append(
                    f"{pybind11_obj_sym}.def_readwrite(\"{cursor.spelling}\",&{self.qualified_name()}::{cursor.spelling});")

        return codes

    def default_pybind11_type_str(self) -> str:
        type_full_name = self.cursor.type.spelling
        return f"pybind11::class_<{type_full_name}>"

    def extra_code(self) -> str:
        """Entity may inject extra code into the generated binding struct."""
        return "\n".join(self.extra_methods_codes)
