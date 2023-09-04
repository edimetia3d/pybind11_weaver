import os.path
from typing import Dict, List
import shutil

from .entity import entity_base
from . import entity_tree
from . import gen_unit

entity_template = """

template <class Pybind11T> struct {bind_struct_name} : public EntityBase {{
  using Pybind11Type = Pybind11T;

  explicit {bind_struct_name}(EntityScope parent_h){{}}

  virtual void Bind(Pybind11T &pb11_obj) {{
    {binding_stmts} 
  }}
  
  {extra_code}
  
  static const char * Key(){{ 
    return {unique_struct_key};
  }}
    
}};


struct {entity_struct_name} : public {bind_struct_name}<std::decay_t<{handle_type}>> {{

  explicit {entity_struct_name}(EntityScope parent_h):
  {bind_struct_name}<std::decay_t<{handle_type}>>(parent_h),
  handle{{ {init_handle_expr} }}
  {{
  }}
  
  void Update() override {{
    Bind(handle);
  }}
  
  EntityScope AsScope() override {{
    return EntityScope(handle);
  }}
  
  {handle_type} handle;
    
}};
"""

file_template = """
// GENERATED AT {date}

{include_directives}

{pybind11_weaver_header}

namespace {{

using pybind11_weaver::EntityScope;
using pybind11_weaver::EntityBase;


{entity_struct_decls}

/**
* Create all entities, return a callable guard that can be called to update all entities.
* If the returned guard is not called, the guard will call the update function on its destruction.
**/
[[nodiscard]] pybind11_weaver::CallUpdateGuard {decl_fn_name}(pybind11::module & m, const pybind11_weaver::CustomBindingRegistry & registry){{
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
        bind_struct_name = "Bind_" + entity.get_cpp_struct_name()
        # generate body
        struct_decl = entity_template.format(
            handle_type=entity.default_pybind11_type_str(),
            entity_struct_name=entity_struct_name,
            bind_struct_name=bind_struct_name,
            parent_expr=parent_sym,
            init_handle_expr=entity.init_default_pybind11_value("parent_h"),
            binding_stmts="\n".join(entity.update_stmts("pb11_obj")),
            unique_struct_key=f"\"{entity.qualified_name()}\"",
            extra_code=entity.extra_code())
        entity_struct_decls.append(struct_decl)

        # generate decl
        create_entity_var_stmts.append(
            f"auto {entity_obj_sym} = pybind11_weaver::CreateEntity<{entity_struct_name}>({parent_sym}, registry);")

        # generate updates
        update_entity_var_stmts.append(f"{entity_obj_sym}->Update();")

        # recursive call to children
        ret = gen_binding_codes(entities[entity.name].children, entity_obj_sym + "->AsScope()", id + 1)
        entity_struct_decls += ret[0]
        create_entity_var_stmts += ret[1]
        update_entity_var_stmts += ret[2]
        id = ret[3]

    return entity_struct_decls, create_entity_var_stmts, update_entity_var_stmts, id


def gen_code(config_file: str):
    gus = gen_unit.load_gen_unit_from_config(config_file)
    for gu in gus:
        # load entities
        entity_root = entity_tree.EntityTree()
        entity_root.load_from_gu(gu)
        target_entities = entity_root.entities
        if gu.options.root_module_namespace != "":
            ns_s = gu.options.root_module_namespace.split("::")
            for ns in ns_s:
                target_entities = target_entities[ns].children
        entity_struct_decls, create_entity_var_stmts, update_entity_var_stmts, _ = gen_binding_codes(
            entities=target_entities,
            parent_sym="EntityScope(m)", beg_id=0)

        # load pybind11_weaver_header
        pybind11_weaver_header_path = os.path.dirname(
            os.path.abspath(__file__)) + "/include/pybind11_weaver/pybind11_weaver.h"
        with open(pybind11_weaver_header_path, "r") as f:
            pybind11_weaver_header = f.read()

        # gen file
        file_content = file_template.format(
            date=gu.creation_time,
            include_directives="\n".join(gu.src_file_includes()),
            pybind11_weaver_header=pybind11_weaver_header,
            decl_fn_name=gu.options.decl_fn_name,
            entity_struct_decls="\n".join(entity_struct_decls),
            create_entity_var_stmts="\n".join(create_entity_var_stmts),
            update_entity_var_stmts="\n".join(update_entity_var_stmts),
        )
        with open(gu.options.output, "w") as f:
            f.write(file_content)

        # format file if clang-format found
        if shutil.which("clang-format") is not None:
            os.system(f"clang-format -i {gu.options.output} --style=LLVM")
