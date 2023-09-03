from typing import Dict, List
from .entity import entity_base
from . import entity_tree
from . import gen_unit

entity_template = """
struct {entity_struct_name} {{
  using HandleT = {handle_type}; 
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


def gen_binding_codes(entities: Dict[str, entity_base.Entity], parent_sym: str, beg_id: int):
    id = beg_id
    entity_struct_decls: List[str] = []
    create_entity_var_stmts: List[str] = []
    update_entity_var_stmts: List[str] = []
    for _, entity in entities.items():
        assert entity is not None
        entity_obj_sym = f"v{id}"
        entity_struct_name = "Entity_" + entity.get_cpp_struct_name()
        # generate body
        struct_decl = entity_template.format(
            handle_type=entity.pybind11_type_str(),
            entity_struct_name=entity_struct_name,
            parent_expr=parent_sym,
            init_handle_expr=entity.create_pybind11_obj_expr("std::forward<ParentT>(parent_h)"),
            binding_stmts="\n".join(entity.update_stmts("handle")))
        entity_struct_decls.append(struct_decl)

        # generate decl
        create_entity_var_stmts.append(
            f"auto {entity_obj_sym} = std::make_shared<{entity_struct_name}>({parent_sym});")

        # generate updates
        update_entity_var_stmts.append(f"{entity_obj_sym}->Update();")

        # recursive call to children
        ret = gen_binding_codes(entities[entity.name].children, entity_obj_sym + "->handle", id + 1)
        entity_struct_decls += ret[0]
        create_entity_var_stmts += ret[1]
        update_entity_var_stmts += ret[2]
        id = ret[3]

    return entity_struct_decls, create_entity_var_stmts, update_entity_var_stmts, id


def gen_code(config_file: str):
    gus = gen_unit.load_gen_unit_from_config(config_file)
    for gu in gus:
        entity_root = entity_tree.EntityTree()
        entity_root.load_from_gu(gu)
        target_entities = entity_root.entities
        if gu.options.root_module_namespace != "":
            ns_s = gu.options.root_module_namespace.split("::")
            for ns in ns_s:
                target_entities = target_entities[ns].children
        entity_struct_decls, create_entity_var_stmts, update_entity_var_stmts, _ = gen_binding_codes(
            entities=target_entities,
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
