import os.path
from typing import Dict, List
import shutil
import sys
import logging

from pybind11_weaver.entity import entity_base, klass, enum
from pybind11_weaver import entity_tree
from pybind11_weaver import gen_unit
from pybind11_weaver.utils import fn, common

_logger = logging.getLogger(__name__)

entity_template = """
{top_level_extra}

template <class Pybind11T={handle_type}> struct {bind_struct_name} : public EntityBase {{
  using Pybind11Type = Pybind11T;
  {extra_code} 
  
  explicit {bind_struct_name}(EntityScope parent_h): handle{{ {init_handle_expr} }}
  {{}}
  
  template<class... HandleArgsT>
  explicit {bind_struct_name}(EntityScope parent_h, HandleArgsT&&... args):handle{{std::forward(args)...}}
  {{}}
  
  void Update() override {{
   {binding_stmts} 
  }}
  
  EntityScope AsScope() override {{
    return EntityScope(handle);
  }}
  
  static const char * Key(){{ 
    return {unique_struct_key};
  }}
   
  Pybind11Type handle; 
}};
#ifndef PB11_WEAVER_DISABLE_{entity_struct_name}

using {entity_struct_name} = {bind_struct_name}<>;

#else

struct {entity_struct_name} : public pybind11_weaver::DisabledEntity {{
  explicit {entity_struct_name}(EntityScope parent_h){{}}
  static const char * Key(){{ 
    return {unique_struct_key};
  }}
}};

#endif // PB11_WEAVER_DISABLE_{entity_struct_name}
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
pybind11_weaver::_PointerWrapperBase::FastBind(m);
{create_warped_pointer_bindings}

{create_entity_var_stmts}

    auto update_fn = [=](){{
{update_entity_var_stmts}    
    }};
    return {{update_fn}};
}}

}} // anonymous namespace

"""


def gen_binding_codes(entities: Dict[str, entity_base.Entity], parent_sym: str, beg_id: int,
                      generated_entities: Dict[str, entity_base.Entity]):
    next_id = beg_id
    entity_struct_decls: List[str] = []
    create_entity_var_stmts: List[str] = []
    update_entity_var_stmts: List[str] = []
    exported_type: List[str] = []
    sorted_keys = sorted(entities.keys())
    used_keys = set()
    while len(used_keys) != len(sorted_keys):
        for key in sorted_keys:
            entity = entities[key]
            bypass = False
            for d in entity.dependency():
                if d not in generated_entities:
                    bypass = True
            if bypass:
                continue
            if key in used_keys:
                continue
            used_keys.add(key)
            generated_entities[entity.reference_name()] = entity
            assert entity is not None
            if isinstance(entity, klass.ClassEntity) or isinstance(entity, enum.EnumEntity):
                exported_type.append(common.safe_type_reference(common.remove_const_ref_pointer(entity.cursor.type)))
            entity_obj_sym = f"v{next_id}"
            entity_struct_name = "Entity_" + entity.get_pb11weaver_struct_name()
            # generate body
            struct_decl = entity_template.format(
                handle_type=entity.default_pybind11_type_str(),
                entity_struct_name=entity_struct_name,
                bind_struct_name="Bind_" + entity.get_pb11weaver_struct_name(),
                parent_expr=parent_sym,
                init_handle_expr=entity.init_default_pybind11_value("parent_h"),
                binding_stmts="\n".join(entity.update_stmts("handle")),
                unique_struct_key=f"\"{entity.get_pb11weaver_struct_name()}\"",
                extra_code=entity.extra_code(),
                top_level_extra=entity.top_level_extra_code())
            entity_struct_decls.append(struct_decl)

            # generate decl
            create_entity_var_stmts.append(
                f"auto {entity_obj_sym} = pybind11_weaver::CreateEntity<{entity_struct_name}>({parent_sym}, registry);")

            # generate updates
            update_entity_var_stmts.append(f"{entity_obj_sym}->Update();")

            # recursive call to children
            ret = gen_binding_codes(entities[entity.name].children, entity_obj_sym + "->AsScope()", next_id + 1,
                                    generated_entities)
            entity_struct_decls += ret[0]
            create_entity_var_stmts += ret[1]
            update_entity_var_stmts += ret[2]
            exported_type += ret[3]
            next_id = ret[4]

    return entity_struct_decls, create_entity_var_stmts, update_entity_var_stmts, exported_type, next_id


def gen_code(config_file: str):
    gus = gen_unit.load_all_gu(config_file)
    for gu in gus:
        # load entities
        entity_root = entity_tree.EntityTree(gu)
        target_entities = entity_root.entities
        if gu.io_config.root_module_namespace != "":
            ns_s = gu.io_config.root_module_namespace.split("::")
            for ns in ns_s:
                target_entities = target_entities[ns].children
        generated_entities = dict()
        entity_struct_decls, create_entity_var_stmts, update_entity_var_stmts, exported_type, _ = gen_binding_codes(
            entities=target_entities,
            parent_sym="EntityScope(m)", beg_id=0, generated_entities=generated_entities)

        warn_unexported_types(exported_type)

        # load pybind11_weaver_header
        pybind11_weaver_header_path = os.path.dirname(
            os.path.abspath(__file__)) + "/include/pybind11_weaver/pybind11_weaver.h"
        with open(pybind11_weaver_header_path, "r") as f:
            pybind11_weaver_header = f.read()

        # gen file
        file_content = file_template.format(
            date=gu.creation_time,
            include_directives="\n".join(gu.include_directives()),
            pybind11_weaver_header=pybind11_weaver_header,
            decl_fn_name=gu.io_config.decl_fn_name,
            entity_struct_decls="\n".join(entity_struct_decls),
            create_warped_pointer_bindings=gen_wrapped_pointer_code(),
            create_entity_var_stmts="\n".join(create_entity_var_stmts),
            update_entity_var_stmts="\n".join(update_entity_var_stmts),
        )
        with open(gu.io_config.output, "w") as f:
            f.write(file_content)

        # format file if clang-format found
        if shutil.which("clang-format") is not None:
            os.system(f"clang-format -i {gu.io_config.output} --style=LLVM")


def gen_wrapped_pointer_code() -> str:
    wrapped_pointer_t = fn.get_wrapped_types()
    wrapped_types = set()
    for t in wrapped_pointer_t:
        s = common.safe_type_reference(t)
        wrapped_types.add(s)
    create_warped_pointer_bindings = []
    for type in sorted(wrapped_types):
        wrapped_type_binding_code_template = "pybind11_weaver::PointerWrapper<{type}>::FastBind(m,\"{safe_type_name}\");"
        safe_type_name = common.type_python_name(type)
        create_warped_pointer_bindings.append(
            wrapped_type_binding_code_template.format(type=type, safe_type_name=safe_type_name))
    create_warped_pointer_bindings = "\n".join(create_warped_pointer_bindings)
    return create_warped_pointer_bindings


def warn_unexported_types(exported_types: List[str]) -> str:
    wrapped = set()
    for t in fn.get_wrapped_types():
        s = common.safe_type_reference(common.remove_const_ref_pointer(t))
        wrapped.add(s)
    used_types = common.get_used_types()
    for t in sorted(used_types):
        if t in exported_types:
            continue
        if t in wrapped:
            continue
        _logger.warning(f"Type `{t}` is used but not exported, some API may not be available. ")
