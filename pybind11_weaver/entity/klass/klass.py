from typing import List, Dict
import logging

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

    def is_pubic(self, cursor: cindex.Cursor):
        return cursor.access_specifier == cindex.AccessSpecifier.CX_CXXPublic and not cursor.is_deleted_method() and common.is_visible(
            cursor, self.gu.io_config.strict_visibility_mode)

    def update_stmts(self, pybind11_obj_sym: str) -> List[str]:
        codes = self._gen_ctor(pybind11_obj_sym)

        # generate method binding
        new_codes, new_extra = method.GenMethod(self).run(pybind11_obj_sym)
        codes.extend(new_codes)
        self.extra_methods_codes.extend(new_extra)

        # generate field binding
        new_codes, new_extra = field.GenFiled(self).run(pybind11_obj_sym)
        codes.extend(new_codes)
        self.extra_methods_codes.extend(new_extra)

        return codes

    def _gen_ctor(self, pybind11_obj_sym: str):
        codes = []
        ctor_found = False
        for cursor in self.cursor.get_children():
            if cursor.kind == cindex.CursorKind.CXCursor_Constructor:
                if self.is_pubic(cursor) and not (cursor.is_move_constructor() or cursor.is_copy_constructor()):
                    param_types = [common.safe_type_reference(arg.type) for arg in cursor.get_arguments()]
                    ctor_found = True
                    codes.append(
                        f"{pybind11_obj_sym}.def(pybind11::init<{','.join(param_types)}>());")
                    if self.gu.io_config.gen_docstring:
                        codes[-1] = entity_base._inject_docstring(codes[-1], cursor, "last_arg")
        if not ctor_found:
            codes.append(f"pybind11_weaver::TryAddDefaultCtor<{self.reference_name()}>({pybind11_obj_sym});")
        return codes

    def default_pybind11_type_str(self) -> str:
        t_param_list = [self.reference_name()]
        base_class = None
        for cursor in self.cursor.get_children():
            if cursor.kind == cindex.CursorKind.CXCursor_CXXBaseSpecifier:
                if base_class is not None:
                    base_class = None
                    _logger.warning(
                        f"Multiple inheritance not supported `{self.cursor.type.spelling}`, base class ignored")
                    break
                base_class = cursor.type
        if base_class is not None:
            common.add_used_types(base_class)
            t_param_list.append(common.safe_type_reference(base_class))
        return f"pybind11::class_<{','.join(t_param_list)}>"

    def extra_code(self) -> str:
        """Entity may inject extra code into the generated binding struct."""
        return "\n".join(self.extra_methods_codes)
