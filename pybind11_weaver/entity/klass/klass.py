from typing import List
import logging

import pylibclang._C
from pylibclang import cindex

from pybind11_weaver.entity import entity_base
from pybind11_weaver.utils import common, scope_list
from pybind11_weaver import gen_unit
from pybind11_weaver.entity.klass import method, field

_logger = logging.getLogger(__name__)


class ClassEntity(entity_base.Entity):

    def __init__(self, gu: gen_unit.GenUnit, cursor: cindex.Cursor):
        entity_base.Entity.__init__(self, gu, cursor)
        assert cursor.kind in [cindex.CursorKind.CXCursor_ClassDecl, cindex.CursorKind.CXCursor_StructDecl]
        self.extra_methods_codes = []

    @property
    def name(self):
        return common.type_python_name(self.cursor.displayname)

    def reference_name(self) -> str:
        return self.cursor.type.spelling

    def get_pb11weaver_struct_name(self) -> str:
        return common.type_python_name(scope_list.get_full_qualified_name(self.cursor))

    def init_default_pybind11_value(self, parent_scope_sym: str) -> str:
        code = f'{parent_scope_sym},"{self.name}", pybind11::dynamic_attr()'
        if self.gu.io_config.gen_docstring:
            code = entity_base._inject_docstring(code, self.cursor, "append")
        return code

    def could_export(self, cursor: cindex.Cursor):
        if common.is_concreate_template(cursor):
            template_cursor = pylibclang._C.clang_getSpecializedCursorTemplate(cursor)
            template_cursor._tu = cursor._tu  # keep compatible with cindex and keep tu alive
            cursor = template_cursor

        return common.is_public(cursor) and not cursor.is_deleted_method() and common.is_visible(
            cursor, self.gu.io_config.strict_visibility_mode)

    def update_stmts(self, pybind11_obj_sym: str) -> List[str]:

        # generate method binding first, because it will inject template argument using decl
        codes, new_extra = method.GenMethod(self).run(pybind11_obj_sym)
        self.extra_methods_codes.extend(new_extra)

        new_codes, new_extra = self._gen_ctor(pybind11_obj_sym)
        codes.extend(new_codes)
        self.extra_methods_codes.extend(new_extra)

        new_codes, new_extra = field.GenFiled(self).run(pybind11_obj_sym)
        codes.extend(new_codes)
        self.extra_methods_codes.extend(new_extra)

        return codes

    def _gen_ctor(self, pybind11_obj_sym: str):
        codes = []
        extra = []

        def add_ctor(cursor, pybind11_obj_sym: str, id: int):
            param_types = [arg.type for arg in cursor.get_arguments()]
            disable_mark = f"PB11_WEAVER_DISABLE_{self.get_pb11weaver_struct_name()}_Ctor{id}"
            if common.is_types_has_unique_ptr(param_types):
                disable_bind = f"#define {disable_mark}"
            else:
                disable_bind = ""
            codes.append(f"AddCtor{id}();")
            param_types = [common.safe_type_reference(arg.type) for arg in cursor.get_arguments()]
            comment = self.cursor.raw_comment
            should_add = self.gu.io_config.gen_docstring and comment is not None
            comment = f'R"_pb11_weaver({comment})_pb11_weaver"' if comment else "nullptr"
            new_extra = f"""
virtual const char * AddCtor{id}(){{
    const char * _pb11_weaver_comment_str = {comment};
    {disable_bind}
#ifndef {disable_mark}
    {pybind11_obj_sym}.def(pybind11::init<{','.join(param_types)}>(){',_pb11_weaver_comment_str' if should_add else ''});
#endif
    return _pb11_weaver_comment_str;
}}
"""
            extra.append(new_extra)

        for cursor in self.cursor.get_children():
            if cursor.kind == cindex.CursorKind.CXCursor_Constructor:
                if self.could_export(cursor) and not (cursor.is_move_constructor() or cursor.is_copy_constructor()):
                    add_ctor(cursor, pybind11_obj_sym, len(codes))

        if len(codes) == 0:
            codes.append(f"pybind11_weaver::TryAddDefaultCtor<{self.reference_name()}>({pybind11_obj_sym});")
        return codes, extra

    def default_pybind11_type_str(self) -> str:
        t_param_list = [self.reference_name()]
        base_class = None
        for cursor in self.cursor.get_children():
            if cursor.kind == cindex.CursorKind.CXCursor_CXXBaseSpecifier:
                if base_class is not None:
                    base_class = None
                    _logger.warning(
                        f"Multiple inheritance not supported `{self.cursor.type.spelling}`, base class ignored")
                else:
                    base_class = cursor.type
            if cursor.kind == cindex.CursorKind.CXCursor_Destructor and not self.could_export(cursor):
                t_param_list.append(f"std::unique_ptr<{self.reference_name()},pybind11::nodelete>")
        if base_class is not None and self.could_export(base_class.get_declaration()):
            common.add_used_types(base_class)
            t_param_list.append(common.safe_type_reference(base_class))
        return f"pybind11::class_<{','.join(t_param_list)}>"

    def extra_code(self) -> str:
        """Entity may inject extra code into the generated binding struct."""
        return "\n".join(self.extra_methods_codes)
