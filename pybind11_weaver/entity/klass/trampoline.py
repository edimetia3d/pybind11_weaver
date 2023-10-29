from typing import Union, Set, List

from pylibclang import cindex

from pybind11_weaver.utils import common

_tramp_method = """
#ifndef PYBIND11_DISABLE_OVERRIDE_{disable_mark}
    {ret_t} {method_name}({params}) {qualifier} override {{
        using _PB11_WR_RET_TYPE = {ret_t};
        using _PB11_WR_CONCREATE_TYPE = {concreate_ref};
        {override_type}(
            _PB11_WR_RET_TYPE, 
            _PB11_WR_CONCREATE_TYPE,
            {method_name}, 
            {args}
            );
    }}
#endif // PYBIND11_DISABLE_OVERRIDE_{disable_mark}
"""

_trampoline = """
template<class = void>
class PyTramp{python_name}{nest_level} : public {base_ref_name} {{
public:
    using _PB11_WR_BaseT = {base_ref_name};
    using _PB11_WR_BaseT::_PB11_WR_BaseT;
    {decls}
    {tramp_methods}
}};
"""


class Virtuals:
    def __init__(self):
        self.virtuals: List[cindex.Cursor] = []
        self.pure_virtuals: List[cindex.Cursor] = []

        self.sigs: Set[str] = set()
        self.template_decls: List[str] = []
        self.subs = dict()

        self.base: Union[Virtuals, None] = None

    def create_base(self):
        assert self.base is None
        self.base = Virtuals()
        self.base.sigs = self.sigs
        return self.base

    def is_empty(self):
        if self.base is not None:
            base_empty = self.base.is_empty()
        else:
            base_empty = True
        return base_empty and len(self.virtuals) == 0 and len(self.pure_virtuals) == 0

    def add_pure_virtual(self, cursor: cindex.Cursor):
        if self._try_add_sig(cursor):
            self.pure_virtuals.append(cursor)

    def add_virtual(self, cursor: cindex.Cursor):
        if self._try_add_sig(cursor):
            self.virtuals.append(cursor)

    def force_add_sig(self, cursor: cindex.Cursor):
        self.sigs.add(self._get_sig(cursor))

    def _get_sig(self, cursor: cindex.Cursor):
        ret_t = common.safe_type_reference(cursor.result_type, self.subs)
        arg_t = [common.safe_type_reference(arg.type, self.subs) for arg in cursor.get_arguments()]
        fn_name = cursor.spelling
        sig = f"{ret_t} {fn_name}({','.join(arg_t)})"
        return sig

    def _try_add_sig(self, cursor: cindex.Cursor):
        sig = self._get_sig(cursor)
        if sig in self.sigs:
            return False
        self.sigs.add(sig)
        return True


class Trampoline:

    def __new__(cls, entity):
        cursor = entity.cursor
        if common.is_marked_final(entity.cursor):
            return None
        virt = Virtuals()
        cls.detect_all_virtual_methods(cursor, virt)
        if virt.is_empty():
            return None
        obj = super().__new__(cls)
        obj._virt = virt
        return obj

    def __init__(self, entity):
        self.entity = entity

    def _get_method(self, cursor: cindex.Cursor, override_type: str, concreate_ref: str):
        ret_t = common.safe_type_reference(cursor.result_type)
        method_name = cursor.spelling
        params_t = [f"{common.safe_type_reference(p.type)}" for p in cursor.get_arguments()]
        args = [p.spelling if p.spelling != "" else f"arg{i}" for i, p in enumerate(cursor.get_arguments())]
        last_right_paren = cursor.type.spelling.rfind(")")
        qualifier = cursor.type.spelling[last_right_paren + 1:]
        return _tramp_method.format(
            disable_mark=common.type_python_name(concreate_ref + cursor.type.spelling),
            ret_t=ret_t,
            method_name=method_name,
            params=", ".join(f"{p_t} {a}" for p_t, a in zip(params_t, args)),
            qualifier=qualifier,
            override_type=override_type,
            concreate_ref=concreate_ref,
            args=", ".join(args)
        )

    def get_virt_def(self, virt: Virtuals, nest_level: int) -> str:
        ret = ""
        python_name = self.entity.name
        concreate_ref = self.entity.reference_name()

        def get_nest_str(nest_level: int) -> str:
            return "" if nest_level == 0 else str(nest_level)

        if virt.base is not None and not virt.base.is_empty():
            ret = self.get_virt_def(virt.base, nest_level + 1)
            base_ref_name = f"PyTramp{self.entity.name}{get_nest_str(nest_level + 1)}<>"
        else:
            base_ref_name = self.entity.reference_name()
        methods = []
        for cursor in virt.virtuals:
            methods.append(self._get_method(cursor, "PYBIND11_OVERRIDE", concreate_ref))
        for cursor in virt.pure_virtuals:
            methods.append(self._get_method(cursor, "PYBIND11_OVERRIDE_PURE", concreate_ref))
        ret = ret + _trampoline.format(
            python_name=python_name,
            base_ref_name=base_ref_name,
            decls="\n".join(virt.template_decls),
            tramp_methods="\n".join(methods),
            nest_level=get_nest_str(nest_level)
        )
        return ret

    def get_defs(self) -> str:
        return self.get_virt_def(self._virt, 0)

    def get_trampoline_cls_name(self):
        return f"PyTramp{self.entity.name}<>"

    @staticmethod
    def detect_all_virtual_methods(cursor: cindex.Cursor, to_update: Virtuals):
        cursor, decls, subs = common.get_def_cls_cursor(cursor)
        if decls is None:
            return
        to_update.template_decls = decls
        to_update.subs = subs
        base_found = None
        for c in cursor.get_children():
            if c.kind == cindex.CursorKind.CXCursor_CXXMethod:
                if c.is_virtual_method():
                    if common.is_marked_final(c) or not common.could_member_accessed(c):
                        to_update.force_add_sig(c)
                        continue
                if not common.could_member_accessed(c):
                    continue
                if c.is_pure_virtual_method():
                    to_update.add_pure_virtual(c)
                elif c.is_virtual_method():
                    to_update.add_virtual(c)
            # recurse into base class
            elif c.kind == cindex.CursorKind.CXCursor_CXXBaseSpecifier:
                assert base_found is None, "Multiple inheritance not supported"
                base_found = c.type.get_declaration()
        if base_found is not None:
            base_virt = to_update.create_base()
            Trampoline.detect_all_virtual_methods(base_found, base_virt)
