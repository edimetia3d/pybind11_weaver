import abc
import functools
import weakref
from typing import List, Dict

from pylibclang import cindex

from pybind11_weaver import gen_unit
from pybind11_weaver.utils import scope_list


def _inject_docstring(code: str, cursor: cindex.Cursor, insert_mode: str):
    if not cursor.raw_comment:
        return code
    if insert_mode == "append":
        code += f',R"_pb11_weaver({cursor.raw_comment})_pb11_weaver"'
    if insert_mode == "last_arg":
        pos = code.rfind(")")
        code = code[:pos] + f',R"_pb11_weaver({cursor.raw_comment})_pb11_weaver"' + code[pos:]
    return code


class Entity(abc.ABC):
    """Entity is like an AST Node.

    Parent is responsible to manage the life cycle of its children, and children only hold a weakref to parent.

    """

    def __init__(self, gu: gen_unit.GenUnit, cursor: cindex.Cursor):
        self.gu = gu
        self.cursor = cursor
        self._parent = None
        self.children: Dict[str, Entity] = {}

    def __contains__(self, item):
        return item in self.children

    def __getitem__(self, item: str):
        return self.children[item]

    def __setitem__(self, key, value):
        self.children[key] = value

    def update_parent(self, parent: "Entity"):
        if self._parent:
            assert self._parent() is parent
        self._parent = weakref.ref(parent)
        if self.name not in parent:
            parent.children[self.name] = self

    def parent(self):
        return self._parent()

    @property
    @functools.lru_cache
    def name(self):
        return self.cursor.spelling

    @functools.lru_cache
    def qualified_name(self) -> str:
        return scope_list.get_full_qualified_name(self.cursor)

    def get_scope(self) -> List[str]:
        return scope_list.get_full_qualified_scopes(self.cursor)

    @abc.abstractmethod
    def get_cpp_struct_name(self) -> str:
        """Unique name of the entity, must be able to used as a C++ identifier."""
        pass

    @abc.abstractmethod
    def init_default_pybind11_value(self, parent_scope_sym: str) -> str:
        """ An expression to create pybind 11 object on stack.
        
        Args:
            parent_scope_sym: the module var name this entity should bind to

        """
        pass

    @abc.abstractmethod
    def update_stmts(self, pybind11_obj_sym: str) -> str:
        """ A group of statements to update the pybind11 object created by codes in `create_pybind11_obj_expr()`

        Args:
            pybind11_obj_sym: the pybind11 object var name

        Returns:

        """
        pass

    @abc.abstractmethod
    def default_pybind11_type_str(self) -> str:
        """ Full type of the pybind11 object."""
        pass

    def extra_code(self) -> str:
        """Entity may inject extra code into the generated binding struct."""
        return ""
