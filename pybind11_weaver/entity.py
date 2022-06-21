from typing import List, Dict

from clang import cindex

from pybind11_weaver import entity_base
from pybind11_weaver import gen_unit
from pybind11_weaver.gen import enum


class EntityManager:
    class EntityEntry:
        def __init__(self, name: str, entity: entity_base.EntityBase = None, qualified_name: str = None):
            self.name = name
            self.entity: entity_base.EntityBase = entity
            self.qualified_name: str = qualified_name
            self.children: Dict[str, EntityManager.EntityEntry] = {}

    def __init__(self):
        self.__entities: Dict[str, EntityManager.EntityEntry] = {}

    def reg(self, entity: entity_base.EntityBase) -> None:
        scopes = entity.get_scope().scopes
        target_scope = self.__entities
        name_history = []
        # Inner scope may be reg before outer scope e.g. MyCls::MyEnum may get reg before `MyCls`
        # So we need to create all outer scope when they do not exist.
        for name in scopes:
            name_history.append(name)
            if name not in target_scope:
                target_scope[name] = EntityManager.EntityEntry(name, None, "::".join(name_history))
            target_scope = target_scope[name].children

        # For the scope issue before, the entry may already be created before
        spelling = entity.get_spelling()
        if spelling not in target_scope:
            target_scope[spelling] = EntityManager.EntityEntry(spelling, entity, "::".join(name_history))
        else:
            assert target_scope[spelling].entity is None, "Entity already registered: {}".format(
                entity.get_scope().str() + "::" + spelling)
            target_scope[spelling].entity = entity

    def load_from_gu(self, gu: gen_unit.GenUnit) -> None:
        root_cursor = gu.tu.cursor
        valid_file_tail_names = gu.src_file_tail_names()
        for cursor in root_cursor.walk_preorder():
            if not check_valid_cursor(cursor, valid_file_tail_names):
                continue
            new_entity = create_entity(cursor)
            if new_entity is not None:
                self.reg(new_entity)

    def entities(self) -> Dict[str, "EntityManager.EntityEntry"]:
        return self.__entities


def create_entity(cursor: cindex.Cursor):
    kind = cursor.kind
    KIND = cindex.CursorKind
    if kind == KIND.ENUM_DECL:
        return enum.EnumEntity(cursor)
    return None


def check_valid_cursor(cursor: cindex.Cursor, valid_tail_names: List[str]):
    file = cursor.location.file
    if file is None:
        return False
    cursor_filename = file.name
    in_src = False
    for tail in valid_tail_names:
        if cursor_filename.endswith(tail):
            in_src = True
            break
    return in_src and cursor.is_definition() and cursor.linkage == cindex.LinkageKind.EXTERNAL
