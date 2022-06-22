
#include <pybind11/pybind11.h>

#include "sample.cc.inc"

PYBIND11_MODULE(enum_module, m) {
  auto update_guard = DeclEnums(m);
}