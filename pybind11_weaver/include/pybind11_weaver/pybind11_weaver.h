#ifndef GITHUB_COM_PYBIND11_WEAVER
#define GITHUB_COM_PYBIND11_WEAVER

#include <functional>
#include <vector>
#include <set>

#include <pybind11/pybind11.h>

namespace pybind11_weaver {

using EntityBindingFn = std::function<void(pybind11::module_ *)>;

/**
 * An Entity is the interface to create a pybind11-binding into a module
 */
struct Entity {
  std::string name;
  EntityBindingFn bind_fn;
  std::vector<std::string> deps_name;
  std::string target_submodule = "";
};

/**
 * Sort all entities into a topological order, so when `entity->bind_fn` is called, all its deps will have been bind
 * already.
 * @param all_entities A set of entities
 * @return sorted entities
 */
template<class = void>
std::vector<Entity *> TopoSort(const std::set<Entity *> &all_entities);
}
#endif // GITHUB_COM_PYBIND11_WEAVER
