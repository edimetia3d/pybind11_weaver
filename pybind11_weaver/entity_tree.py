from typing import List, Dict
import weakref

from clang import cindex

from pybind11_weaver import gen_unit
from pybind11_weaver.entity import create_entity
from pybind11_weaver.entity import entity_base
from pybind11_weaver.entity import funktion, klass

from pybind11_weaver.utils import common


class _DummyNode(entity_base.Entity):

    def __init__(self, gu: gen_unit.GenUnit, cursor: cindex.Cursor):
        entity_base.Entity.__init__(self, gu, cursor)

    def transfer(self, new_entity: "Entity"):
        # update parent
        self.parent().children[self.name] = new_entity

        # update children
        for child in self.children:
            assert child.parent() is self
            child._parent = weakref.ref(new_entity)
        new_entity.children = self.children

    def get_cpp_struct_name(self) -> str:
        raise NotImplementedError

    def init_default_pybind11_value(self, parent_scope_sym: str) -> str:
        raise NotImplementedError

    def update_stmts(self, pybind11_obj_sym: str) -> List[str]:
        raise NotImplementedError

    def default_pybind11_type_str(self) -> str:
        raise NotImplementedError


class EntityTree:
    """
    Entity Tree like an AST tree, itself is the root node.
    """

    def __init__(self):
        self.entities: Dict[str, entity_base.Entity] = {}

    def nest_update_parent(self, entity: entity_base.Entity) -> None:
        scopes = entity.get_scope()
        # try create/update parent
        outer = self.entities
        for name in scopes:
            if name not in outer:
                outer[name] = _DummyNode(entity.gu, None)
                if not outer is self.entities:
                    outer[name].update_parent(outer)
            outer = outer[name]

        # setup entity
        entity_name = entity.name
        if entity_name in outer:
            if isinstance(outer[entity_name], _DummyNode):
                outer[entity_name].transfer(entity)
            else:
                if isinstance(entity, funktion.FunctionEntity):
                    assert isinstance(outer[entity_name], funktion.FunctionEntity)
                    outer[entity_name].overloads.append(entity.cursor)
        else:
            outer[entity_name] = entity
        if not outer is self.entities:
            entity.update_parent(outer)

    def load_from_gu(self, gu: gen_unit.GenUnit) -> None:
        root_cursor = gu.tu.cursor
        valid_file_tail_names = gu.src_file_tail_names()
        for cursor in root_cursor.walk_preorder():
            if not self.check_valid_cursor(cursor, valid_file_tail_names):
                continue
            new_entity = create_entity(gu, cursor)
            if new_entity is not None:
                self.nest_update_parent(new_entity)

    def check_valid_cursor(self, cursor: cindex.Cursor, valid_tail_names: List[str]):
        file = cursor.location.file
        if file is None:
            return False
        cursor_filename = file.name
        in_src = False
        for tail in valid_tail_names:
            if cursor_filename.endswith(tail):
                in_src = True
                break
        return in_src and cursor.linkage == cindex.LinkageKind.EXTERNAL
