import argparse
from typing import Dict

import pybind11_weaver
from pybind11_weaver import entity
from pybind11_weaver import gen_unit


def _handle_optional_args(args):
    if hasattr(args, "get_include") and args.get_include:
        print(pybind11_weaver.get_include())
        exit(0)


def _add_required_args(parser: argparse.ArgumentParser):
    parser.add_argument("--config",
                        type=str,
                        required=True,
                        help="Path to the config file")


def _add_optional_args(parser: argparse.ArgumentParser):
    parser.add_argument("--get_include",
                        action="store_true",
                        default=False,
                        help="Print the pybind11_weaver include path and exit.")


def parse_args():
    parser = argparse.ArgumentParser(description="Pybind11 command line interface.")
    _add_optional_args(parser)
    _handle_optional_args(parser.parse_known_args())
    _add_required_args(parser)
    return parser.parse_args()


ARGS = parse_args()

entity_template = """
template<>
struct pybind11_weaver::Entity<{uuid}> {{
  using HandleT = {handle_type}; // User are free to modify HandleT
  template<class ParentT>
  explicit Entity(ParentT && parent_h):handle{{ {init_handle_expr} }}{{
  }}
  
  void Update(){{
    //Binding codes here
{binding_stmts}
  }}
  HandleT handle;
  static const const char * qualified_name = "{qualified_name}"; // User MUST NOT modify this decl
}};
    """


def create_decl_fn(entities: Dict[str, entity.EntityManager.EntityEntry], parent_sym="m", beg_id=0):
    id = beg_id

    for _, entity_entry in entities.items():
        entity_sym = f"v{id}_{entity_entry.name}"
        init_expr = f"""parent_h.def_submodule("{entity_entry.name}")"""
        type_str = "pybind11::module_"
        binds_stmts = []
        if entity_entry.entity is not None:
            init_expr = entity_entry.entity.declare_expr("std::forward<ParentT>(parent_h)")
            type_str = entity_entry.entity.pybind11_type_str()
            binds_stmts = entity_entry.entity.update_stmts("handle")
        code = entity_template.format(
            handle_type=type_str,
            uuid=id,
            parent_expr=parent_sym,
            init_handle_expr=init_expr,
            binding_stmts="\n".join(binds_stmts),
            qualified_name=entity_entry.qualified_name)
        ## DBG PRINT
        print(code)
        print(
            f"auto {entity_sym} = std::make_shared<pybind11_weaver::Entity<{id}>>({parent_sym});")
        print(f"{entity_sym}->Update();")
        ## DBG PRINT
        id = id + 1
        create_decl_fn(entities[entity_entry.name].children, entity_sym + "->handle", id)


def main():
    gus = gen_unit.load_gen_unit_from_config(ARGS.config)
    for gu in gus:
        entity_mgr = entity.EntityManager()
        entity_mgr.load_from_gu(gu)
        create_decl_fn(entity_mgr.entities())


if __name__ == "__main__":
    main()
