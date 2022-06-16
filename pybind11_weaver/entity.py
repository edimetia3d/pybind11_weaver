from typing import List

from clang import cindex

from pybind11_weaver import gen_unit
from pybind11_weaver.gen import enum


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


def get_all_entities(gu: gen_unit.GenUnit):
    root_cursor = gu.tu.cursor
    entities = []
    valid_file_tail_names = gu.src_file_tail_names()
    for cursor in root_cursor.walk_preorder():
        if not check_valid_cursor(cursor, valid_file_tail_names):
            continue
        new_entity = create_entity(cursor)
        if new_entity is not None:
            entities.append(new_entity)
    return entities
