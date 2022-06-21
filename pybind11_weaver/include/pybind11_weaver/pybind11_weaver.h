#ifndef GITHUB_COM_PYBIND11_WEAVER
#define GITHUB_COM_PYBIND11_WEAVER

#include <string>
#include <map>

#include <pybind11/pybind11.h>

namespace pybind11_weaver {

struct EntityMap {
  static void Set(const std::string &key, pybind11::object &obj) {
    assert(GetMap().count(key) == 0);
    GetMap()[key] = obj;
  }

  static pybind11::object Get(const std::string &key) {
    assert(GetMap().count(key) != 0);
    return GetMap()[key];
  }
  static void Clear() {
    GetMap().clear();
  }

private:
  template<class T = void>
  static std::map<std::string, pybind11::object> &GetMap() {
    static std::map<std::string, pybind11::object> map;
    return map;
  }
};

} // namespace pybind11_weaver
#endif // GITHUB_COM_PYBIND11_WEAVER
