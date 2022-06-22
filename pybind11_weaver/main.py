import argparse
from typing import Dict, List

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
struct {entity_struct_name} {{
  using HandleT = {handle_type}; // User are free to modify HandleT
  template<class ParentT>
  explicit {entity_struct_name}(ParentT && parent_h):handle{{ {init_handle_expr} }}{{
  }}

  {entity_struct_name}({entity_struct_name} &&) = delete;
  {entity_struct_name}(const {entity_struct_name} &) = delete;
  
  void Update(){{
    //Binding codes here
{binding_stmts}
  }}
  HandleT handle;
  [[maybe_unused]] const char * qualified_name = "{qualified_name}"; // User MUST NOT modify this decl
}};
"""

file_template = """
// GENERATED AT {date}

{include_directives}

#include <pybind11_weaver/pybind11_weaver.h>

namespace {{
{entity_struct_decls}

/**
* Create all entities, return a callable guard that can be called to update all entities.
* If the returned guard is not called, the guard will call the update function on its destruction.
**/
[[nodiscard]] pybind11_weaver::CallUpdateGuard {decl_fn_name}(pybind11::module & m){{
{create_entity_var_stmts}

    auto update_fn = [=](){{
{update_entity_var_stmts}    
    }};
    return {{update_fn}};
}}

}} // anonymous namespace

"""


def create_decl_fn(entities: Dict[str, entity.EntityManager.EntityEntry], parent_sym: str, beg_id: int):
    id = beg_id
    entity_struct_decls: List[str] = []
    create_entity_var_stmts: List[str] = []
    update_entity_var_stmts: List[str] = []
    for _, entity_entry in entities.items():
        entity_obj_sym = f"v{id}"
        entity_struct_name = "Entity_" + entity_entry.entity.get_unique_name()
        assert entity_entry.entity is not None
        init_expr = entity_entry.entity.declare_expr("std::forward<ParentT>(parent_h)")
        type_str = entity_entry.entity.pybind11_type_str()
        binds_stmts = entity_entry.entity.update_stmts("handle")
        struct_decl = entity_template.format(
            handle_type=type_str,
            entity_struct_name=entity_struct_name,
            parent_expr=parent_sym,
            init_handle_expr=init_expr,
            binding_stmts="\n".join(binds_stmts),
            qualified_name=entity_entry.qualified_name)
        entity_struct_decls.append(struct_decl)
        create_entity_var_stmts.append(
            f"auto {entity_obj_sym} = std::make_shared<{entity_struct_name}>({parent_sym});")
        update_entity_var_stmts.append(f"{entity_obj_sym}->Update();")
        id = id + 1
        ret = create_decl_fn(entities[entity_entry.name].children, entity_obj_sym + "->handle", id)
        entity_struct_decls += ret[0]
        create_entity_var_stmts += ret[1]
        update_entity_var_stmts += ret[2]

    return entity_struct_decls, create_entity_var_stmts, update_entity_var_stmts


def main():
    gus = gen_unit.load_gen_unit_from_config(ARGS.config)
    for gu in gus:
        entity_mgr = entity.EntityManager()
        entity_mgr.load_from_gu(gu)
        target_entities = entity_mgr.entities()
        if gu.options.root_module_namespace != "":
            ns_s = gu.options.root_module_namespace.split("::")
            for ns in ns_s:
                target_entities = target_entities[ns].children
        entity_struct_decls, create_entity_var_stmts, update_entity_var_stmts = create_decl_fn(entities=target_entities,
                                                                                               parent_sym="m", beg_id=0)
        file_content = file_template.format(
            date=gu.creation_time,
            include_directives="\n".join(gu.src_file_includes()),
            decl_fn_name=gu.options.decl_fn_name,
            entity_struct_decls="\n".join(entity_struct_decls),
            create_entity_var_stmts="\n".join(create_entity_var_stmts),
            update_entity_var_stmts="\n".join(update_entity_var_stmts),
        )
        with open(gu.options.output, "w") as f:
            f.write(file_content)


if __name__ == "__main__":
    main()
