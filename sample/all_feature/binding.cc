
#include <pybind11/pybind11.h>

#include "sample_binding.cc.inc"

PYBIND11_MODULE(all_feature_module, m) { auto update_guard = DeclFn(m); }