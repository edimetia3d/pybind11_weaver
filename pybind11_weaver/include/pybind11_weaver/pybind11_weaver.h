#ifndef GITHUB_COM_PYBIND11_WEAVER
#define GITHUB_COM_PYBIND11_WEAVER

#include <pybind11/pybind11.h>

namespace pybind11_weaver {
using UUID_t = int64_t;
template<UUID_t UUID>
struct Entity {

};

} // namespace pybind11_weaver
#endif // GITHUB_COM_PYBIND11_WEAVER
