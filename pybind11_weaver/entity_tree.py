import logging
from typing import List, Dict, Tuple
from contextlib import contextmanager

from pylibclang import cindex
import pylibclang._C

from pybind11_weaver import gen_unit
from pybind11_weaver.entity import create_entity
from pybind11_weaver.entity import entity_base, funktion

from pybind11_weaver.utils import common

_logger = logging.getLogger(__name__)


@contextmanager
def _inject_tu(nodes, gu):
    for node in nodes:
        node._tu = gu.tu  # keep compatible with cindex and keep tu alive
    yield nodes
    for node in nodes:
        del node._tu


def get_template_struct_class(cursor: cindex.Cursor):
    source_file = cursor.location.file
    source_line = cursor.location.line
    # read the source_line-th line's content in the source file
    with open(source_file.name, 'r') as f:
        source_code = f.read()
        line_content = source_code.splitlines()[source_line - 1]
    if "struct" in line_content:
        return "extern template struct"
    else:
        return "extern template class"


class EntityTree:
    """
    Entity Tree like an AST tree, itself is the root node.
    """

    def __init__(self, gu: gen_unit.GenUnit):
        self.entities: Dict[str, entity_base.Entity] = {}
        self.gu = gu
        self._map_from_gu(gu)

    def add_child(self, child: "Entity"):
        if child.name in self.entities:
            if child.cursor.kind == cindex.CursorKind.CXCursor_Namespace:
                assert len(child.children) == 0
            else:
                _logger.warning(
                    f"Entity at {child.cursor.location} already exists, skip, previous one is {self.entities[child.name].cursor.location}")
        else:
            self.entities[child.name] = child
        assert child.parent() is None

    def __getitem__(self, item):
        return self.entities[item]

    def __contains__(self, item):
        return item in self.entities

    def _inject_explicit_template_instantiation(self, gu: gen_unit.GenUnit):
        explicit_instantiation = set()
        implicit_instantiation = dict()

        def is_valid(cursor: cindex.Cursor):
            return gu.is_cursor_in_inputs(cursor)

        def visitor(cursor, parent, unused1):
            if not is_valid(cursor):
                return pylibclang._C.CXChildVisitResult.CXChildVisit_Continue
            with _inject_tu([cursor, parent], gu):
                if cursor.kind in [cindex.CursorKind.CXCursor_ClassDecl,
                                   cindex.CursorKind.CXCursor_StructDecl] and common.is_concreate_template(cursor):
                    key_name = common.safe_type_reference(cursor.type)
                    explicit_instantiation.add(key_name)
                    if key_name in implicit_instantiation:
                        del implicit_instantiation[key_name]
                elif cursor.kind == cindex.CursorKind.CXCursor_TemplateRef and is_valid(cursor.referenced):
                    # we can not get more info from template ref anymore, so we need to get info from parent
                    possible_types = []
                    if parent.kind in [cindex.CursorKind.CXCursor_FieldDecl, cindex.CursorKind.CXCursor_ParmDecl,
                                       cindex.CursorKind.CXCursor_CXXBaseSpecifier]:
                        possible_types = [parent.type]
                    elif parent.kind in [cindex.CursorKind.CXCursor_FunctionDecl, cindex.CursorKind.CXCursor_CXXMethod]:
                        possible_types = [parent.result_type] + [arg.type for arg in parent.get_arguments()]
                    else:
                        _logger.info(
                            f"Only template instance may used in python will get auto exported, ignored {parent.kind} at {parent.location}")

                    for t in possible_types:
                        t = common.remove_const_ref_pointer(t).get_canonical()
                        t_c = t.get_declaration()
                        if common.is_concreate_template(t_c):
                            template_c = pylibclang._C.clang_getSpecializedCursorTemplate(t_c)
                            if is_valid(template_c):
                                key_name = common.safe_type_reference(t)
                                if key_name not in explicit_instantiation:
                                    implicit_instantiation[key_name] = get_template_struct_class(template_c)

            return pylibclang._C.CXChildVisitResult.CXChildVisit_Recurse

        pylibclang._C.clang_visitChildren(gu.tu.cursor, visitor, pylibclang._C.voidp(0))
        init_code = "\n".join([f"{prefix} {type_name};" for type_name, prefix in implicit_instantiation.items()])
        if len(implicit_instantiation) > 0:
            _logger.info(f"Implicit template instance binding added: \n {init_code}");
        gu.reload_tu(init_code)
        funktion.FunctionEntity._added_func.clear()

    def _map_from_gu(self, gu: gen_unit.GenUnit):
        self._inject_explicit_template_instantiation(gu)
        last_parent: List[entity_base.Entity] = [None]
        worklist: List[Tuple[cindex.Cursor, entity_base.Entity]] = [(gu.tu.cursor, self)]

        def visitor(child_cursor, unused0, unused1):
            child_cursor._tu = gu.tu  # keep compatible with cindex and keep tu alive
            parent = last_parent[0]
            if gu.is_cursor_in_inputs(child_cursor):
                if child_cursor.kind == cindex.CursorKind.CXCursor_UnexposedDecl:
                    worklist.append((child_cursor, parent))
                    return pylibclang._C.CXChildVisitResult.CXChildVisit_Continue
                new_entity = create_entity(gu, child_cursor)
                if new_entity is None:
                    return pylibclang._C.CXChildVisitResult.CXChildVisit_Continue
                parent.add_child(new_entity)
                worklist.append((new_entity.cursor, parent[new_entity.name]))
            return pylibclang._C.CXChildVisitResult.CXChildVisit_Continue

        while len(worklist) > 0:
            new_item = worklist.pop()
            last_parent[0] = new_item[1]
            next_cur = new_item[0]
            pylibclang._C.clang_visitChildren(next_cur, visitor, pylibclang._C.voidp(0))
