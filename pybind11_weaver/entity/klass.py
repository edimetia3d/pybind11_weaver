from typing import List, Dict
import logging

from clang import cindex

from . import entity_base
from pybind11_weaver.utils import fn

from pybind11_weaver import gen_unit

_logger = logging.getLogger(__name__)


class MethodCoder:

    def __init__(self, cursor, scope_full_name, inject_docstring):
        self.inect_docstring = inject_docstring
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

        casted_pointer = fn.get_fn_value_expr(cursor)
        if is_static(cursor):
            self.body.append(
                f"obj.def_static(\"{cursor.spelling}\",{casted_pointer});")
        else:
            self.body.append(
                f"obj.def(\"{cursor.spelling}\",{casted_pointer});")
            if is_virtual(cursor):
                _logger.warning(
                    f"virtual method {cursor.spelling} is not fully supported yet, override in python is not allowed.")
        if self.inect_docstring:
            self.body[-1] = entity_base._inject_docstring(
                self.body[-1], cursor, "last_arg")


def _is_bindable_type(type: cindex.Type):
    type = type.get_canonical()
    if type.kind in [cindex.TypeKind.CONSTANTARRAY, cindex.TypeKind.INCOMPLETEARRAY, cindex.TypeKind.VARIABLEARRAY]:
        return False
    if fn.warp_type(type, "")[0] is not None:
        return False
    return True


class ClassEntity(entity_base.Entity):

    def __init__(self, gu: gen_unit.GenUnit, cursor: cindex.Cursor):
        entity_base.Entity.__init__(self, gu, cursor)
        assert cursor.kind in [cindex.CursorKind.CLASS_DECL, cindex.CursorKind.STRUCT_DECL]
        self.extra_methods_codes = []

    def get_cpp_struct_name(self) -> str:
        return self.cursor.type.spelling.replace("::", "_")

    def init_default_pybind11_value(self, parent_scope_sym: str) -> str:
        code = f'{parent_scope_sym},"{self.name}"'
        if self.gu.io_config.gen_docstring:
            code = entity_base._inject_docstring(code, self.cursor, "append")
        return code

    def update_stmts(self, pybind11_obj_sym: str) -> List[str]:
        codes = []

        def is_pubic(cursor):
            return cursor.access_specifier == cindex.AccessSpecifier.PUBLIC and not cursor.is_deleted_method()

        def not_operator(cursor):
            is_operator = "operator" in cursor.spelling
            if is_operator:
                _logger.warning(f"Operator overloading not supported `{cursor.spelling}`")
            return not is_operator

        # generate constructor binding
        ctor_found = False
        for cursor in self.cursor.get_children():
            if cursor.kind == cindex.CursorKind.CONSTRUCTOR:
                ctor_found = True
                if is_pubic(cursor) and not (cursor.is_move_constructor() or cursor.is_copy_constructor()):
                    param_types = fn.fn_arg_type(cursor)
                    codes.append(
                        f"{pybind11_obj_sym}.def(pybind11::init<{','.join(param_types)}>());")
        if not ctor_found:
            codes.append(f"{pybind11_obj_sym}.def(pybind11::init<>());")
        if self.gu.io_config.gen_docstring:
            for i, code in enumerate(codes):
                codes[i] = entity_base._inject_docstring(code, cursor, "last_arg")

        # generate method binding
        methods: Dict[str, MethodCoder] = dict()
        for cursor in self.cursor.get_children():
            if cursor.kind == cindex.CursorKind.CXX_METHOD and is_pubic(cursor) and not_operator(cursor):
                if not cursor.spelling in methods:
                    methods[cursor.spelling] = MethodCoder(cursor, self.qualified_name(),
                                                           self.gu.io_config.gen_docstring)
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
            if cursor.kind == cindex.CursorKind.FIELD_DECL and \
                    is_pubic(cursor) and \
                    _is_bindable_type(cursor.type):
                filed_binder = "def_readwrite"
                if cursor.type.is_const_qualified():
                    filed_binder = "def_readonly"
                codes.append(
                    f"{pybind11_obj_sym}.{filed_binder}(\"{cursor.spelling}\",&{self.qualified_name()}::{cursor.spelling});")
                if self.gu.io_config.gen_docstring:
                    codes[-1] = entity_base._inject_docstring(codes[-1], cursor, "last_arg")

        return codes

    def default_pybind11_type_str(self) -> str:
        type_full_name = self.cursor.type.spelling
        return f"pybind11::class_<{type_full_name}>"

    def extra_code(self) -> str:
        """Entity may inject extra code into the generated binding struct."""
        return "\n".join(self.extra_methods_codes)
