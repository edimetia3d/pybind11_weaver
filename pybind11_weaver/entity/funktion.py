import collections
from typing import List, Dict

from pylibclang import cindex

from pybind11_weaver import gen_unit
from pybind11_weaver.utils import fn
from . import entity_base


class FunctionEntity(entity_base.Entity):
    _added_func: Dict[str, int] = collections.defaultdict(int)

    def __init__(self, gu: gen_unit.GenUnit, cursor: cindex.Cursor):
        entity_base.Entity.__init__(self, gu, cursor)
        assert cursor.kind == cindex.CursorKind.CXCursor_FunctionDecl
        python_name = fn.fn_python_name(cursor)
        struct_name = python_name
        if FunctionEntity._added_func[python_name] != 0:
            struct_name += str(FunctionEntity._added_func[python_name])
        FunctionEntity._added_func[python_name] += 1
        self._struct_name = struct_name
        self._extra_code = ""

    def reference_name(self) -> str:
        return fn.fn_ref_name(self.cursor)

    def get_pb11weaver_struct_name(self) -> str:
        return self._struct_name

    def init_default_pybind11_value(self, parent_scope_sym: str) -> str:
        return f"static_cast<pybind11::module_&>({parent_scope_sym})"

    def update_stmts(self, pybind11_obj_sym: str) -> List[str]:
        disable_mark = f"PB11_WEAVER_DISABLE_{self.get_pb11weaver_struct_name()}_AddFunction"
        fn_ptr = fn.get_fn_value_expr(self.cursor)
        disable_bind = f"#define {disable_mark}" if fn_ptr is None else ""
        comment = self.cursor.raw_comment
        should_add = self.gu.io_config.gen_docstring and comment is not None
        comment = f'R"_pb11_weaver({comment})_pb11_weaver"' if comment else "nullptr"
        self._extra_code = f"""
virtual const char * AddFunction(){{
    const char * _pb11_weaver_comment_str = {comment};
    {disable_bind}
#ifndef {disable_mark}
    {pybind11_obj_sym}.def(\"{fn.fn_python_name(self.cursor)}\",{fn_ptr}{',_pb11_weaver_comment_str' if should_add else ''});
#endif
    return _pb11_weaver_comment_str;
}}
"""
        return ["AddFunction();"]

    def default_pybind11_type_str(self) -> str:
        return f"pybind11::module_ &"

    def extra_code(self) -> str:
        """Entity may inject extra code into the generated binding struct."""
        return self._extra_code
