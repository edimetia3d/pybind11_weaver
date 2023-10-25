from typing import List, Dict

from pylibclang import cindex
import pylibclang._C

from pybind11_weaver import gen_unit
from pybind11_weaver.entity import create_entity
from pybind11_weaver.entity import entity_base
from pybind11_weaver.entity import funktion

from pybind11_weaver.utils import common


class EntityTree:
    """
    Entity Tree like an AST tree, itself is the root node.
    """

    def __init__(self, gu: gen_unit.GenUnit):
        self.entities: Dict[str, entity_base.Entity] = {}
        self._map_from_gu(gu)
        self.gu = gu

    def add_child(self, child: "Entity"):
        assert child.name not in self.entities
        self.entities[child.name] = child
        assert child.parent() is None

    def __getitem__(self, item):
        return self.entities[item]

    def __contains__(self, item):
        return item in self.entities

    def _inject_explicit_template_instantiation(self, gu: gen_unit.GenUnit):
        mab_be_template_instance = [cindex.CursorKind.CXCursor_ClassDecl,
                                    cindex.CursorKind.CXCursor_StructDecl, cindex.CursorKind.CXCursor_FunctionDecl]
        explicit_instantiation = set()
        implicit_instantiation = dict()
        inc_files = gu.include_files()

        def visitor(cursor, parent, unused1):
            if not self.check_valid_cursor(cursor, inc_files, gu.io_config.strict_visibility_mode):
                return pylibclang._C.CXChildVisitResult.CXChildVisit_Continue
            parent._tu = gu.tu  # keep compatible with cindex and keep tu alive
            cursor._tu = gu.tu
            if cursor.kind in mab_be_template_instance and common.is_concreate_template(cursor):
                explicit_instantiation.add(cursor.displayname)
                if cursor.displayname in implicit_instantiation:
                    del implicit_instantiation[cursor.displayname]
            elif cursor.kind == cindex.CursorKind.CXCursor_TemplateRef:
                concreate_t_name = None
                if parent.kind in [cindex.CursorKind.CXCursor_FieldDecl, cindex.CursorKind.CXCursor_VarDecl,
                                   cindex.CursorKind.CXCursor_ParmDecl]:
                    concreate_t_name = parent.type.spelling
                elif parent.kind == cindex.CursorKind.CXCursor_CXXBaseSpecifier:
                    concreate_t_name = parent.displayname
                elif parent.kind == cindex.CursorKind.CXCursor_FunctionDecl:
                    concreate_t_name = parent.result_type.spelling
                assert concreate_t_name
                struct_kind = pylibclang._C.clang_getTemplateCursorKind(cursor.referenced)
                explict_prefix = None
                if struct_kind == cindex.CursorKind.CXCursor_StructDecl:
                    explict_prefix = "template struct"
                elif struct_kind == cindex.CursorKind.CXCursor_ClassDecl:
                    explict_prefix = "template class"
                if concreate_t_name not in explicit_instantiation:
                    implicit_instantiation[concreate_t_name] = explict_prefix
            del cursor._tu
            del parent._tu
            return pylibclang._C.CXChildVisitResult.CXChildVisit_Recurse

        pylibclang._C.clang_visitChildren(gu.tu.cursor, visitor, pylibclang._C.voidp(0))
        init_code = "\n".join([f"{prefix} {type_name};" for prefix, type_name in implicit_instantiation.items()])
        gu.reload_tu(init_code)

    def _map_from_gu(self, gu: gen_unit.GenUnit):
        self._inject_explicit_template_instantiation(gu)
        valid_files = gu.include_files() + [gu.unsaved_file[0]]
        visibility_mode = gu.io_config.strict_visibility_mode
        last_parent: List[entity_base.Entity] = [self]
        worklist: List[entity_base.Entity] = []

        def visitor(child_cursor, unused0, unused1):
            child_cursor._tu = gu.tu  # keep compatible with cindex and keep tu alive
            parent = last_parent[0]
            if self.check_valid_cursor(child_cursor, valid_files, visibility_mode):
                new_entity = create_entity(gu, child_cursor)
                if new_entity is None:
                    return pylibclang._C.CXChildVisitResult.CXChildVisit_Continue
                if new_entity.name in parent:
                    # overloading
                    assert isinstance(new_entity, funktion.FunctionEntity)
                    assert isinstance(parent[new_entity.name], funktion.FunctionEntity)
                    parent[new_entity.name].overloads.append(child_cursor)
                else:
                    parent.add_child(new_entity)
                worklist.append(new_entity)
            return pylibclang._C.CXChildVisitResult.CXChildVisit_Continue

        pylibclang._C.clang_visitChildren(gu.tu.cursor, visitor, pylibclang._C.voidp(0))
        while len(worklist) > 0:
            new_parent = worklist.pop()
            last_parent[0] = new_parent
            pylibclang._C.clang_visitChildren(new_parent.cursor, visitor, pylibclang._C.voidp(0))

    def check_valid_cursor(self, cursor: cindex.Cursor, valid_tail_names: List[str], strict_visibility_mode: bool):
        file = cursor.location.file
        if file is None:
            return False
        cursor_filename = file.name
        in_src = False
        for tail in valid_tail_names:
            if cursor_filename.endswith(tail):
                in_src = True
                break
        return in_src and cursor.linkage != cindex.LinkageKind.CXLinkage_Internal and common.is_visible(cursor,
                                                                                                        strict_visibility_mode)
