
#include <pybind11/pybind11.h>

#include "sample_binding.cc.inc"

PYBIND11_MODULE(all_feature_module, m) {
  pybind11_weaver::CustomBindingRegistry reg;
  reg.DisableBinding<Entity_disabled_space>();
  reg.DisableBinding<Entity_disabled_member_disabled_Foo>();
  auto update_guard = DeclFn(m, reg);
}