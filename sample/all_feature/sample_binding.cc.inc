
// GENERATED AT 09/03/2023, 22:31:01

#include "sample.h"

#include <pybind11_weaver/pybind11_weaver.h>

namespace {

struct Entity_TopFunction {
  using HandleT = pybind11::module_ &;
  template <class ParentT>
  explicit Entity_TopFunction(ParentT &&parent_h)
      : handle{std::forward<ParentT>(parent_h)} {}

  Entity_TopFunction(Entity_TopFunction &&) = delete;
  Entity_TopFunction(const Entity_TopFunction &) = delete;

  void Update() {
    // Binding codes here
    handle.def("TopFunction", static_cast<void (*)(int)>(&TopFunction));
    handle.def("TopFunction",
               static_cast<void (*)(std::string &)>(&TopFunction));
  }
  HandleT handle;
};

struct Entity_TopFunctionDef {
  using HandleT = pybind11::module_ &;
  template <class ParentT>
  explicit Entity_TopFunctionDef(ParentT &&parent_h)
      : handle{std::forward<ParentT>(parent_h)} {}

  Entity_TopFunctionDef(Entity_TopFunctionDef &&) = delete;
  Entity_TopFunctionDef(const Entity_TopFunctionDef &) = delete;

  void Update() {
    // Binding codes here
    handle.def("TopFunctionDef", static_cast<void (*)()>(&TopFunctionDef));
  }
  HandleT handle;
};

struct Entity_earth {
  using HandleT = pybind11::module_;
  template <class ParentT>
  explicit Entity_earth(ParentT &&parent_h)
      : handle{std::forward<ParentT>(parent_h).def_submodule("earth")} {}

  Entity_earth(Entity_earth &&) = delete;
  Entity_earth(const Entity_earth &) = delete;

  void Update() {
    // Binding codes here
  }
  HandleT handle;
};

struct Entity_earth_creatures {
  using HandleT = pybind11::module_;
  template <class ParentT>
  explicit Entity_earth_creatures(ParentT &&parent_h)
      : handle{std::forward<ParentT>(parent_h).def_submodule("creatures")} {}

  Entity_earth_creatures(Entity_earth_creatures &&) = delete;
  Entity_earth_creatures(const Entity_earth_creatures &) = delete;

  void Update() {
    // Binding codes here
  }
  HandleT handle;
};

struct Entity_earth_creatures_NSFunction {
  using HandleT = pybind11::module_ &;
  template <class ParentT>
  explicit Entity_earth_creatures_NSFunction(ParentT &&parent_h)
      : handle{std::forward<ParentT>(parent_h)} {}

  Entity_earth_creatures_NSFunction(Entity_earth_creatures_NSFunction &&) =
      delete;
  Entity_earth_creatures_NSFunction(const Entity_earth_creatures_NSFunction &) =
      delete;

  void Update() {
    // Binding codes here
    handle.def("NSFunction", static_cast<int (*)(const std::string &)>(
                                 &earth::creatures::NSFunction));
  }
  HandleT handle;
};

struct Entity_earth_creatures_NSFunctionDef {
  using HandleT = pybind11::module_ &;
  template <class ParentT>
  explicit Entity_earth_creatures_NSFunctionDef(ParentT &&parent_h)
      : handle{std::forward<ParentT>(parent_h)} {}

  Entity_earth_creatures_NSFunctionDef(
      Entity_earth_creatures_NSFunctionDef &&) = delete;
  Entity_earth_creatures_NSFunctionDef(
      const Entity_earth_creatures_NSFunctionDef &) = delete;

  void Update() {
    // Binding codes here
    handle.def("NSFunctionDef",
               static_cast<void (*)()>(&earth::creatures::NSFunctionDef));
  }
  HandleT handle;
};

struct Entity_earth_creatures_Animal {
  using HandleT = pybind11::enum_<earth::creatures::Animal>;
  template <class ParentT>
  explicit Entity_earth_creatures_Animal(ParentT &&parent_h)
      : handle{pybind11::enum_<earth::creatures::Animal>(
            std::forward<ParentT>(parent_h), "Animal",
            pybind11::arithmetic())} {}

  Entity_earth_creatures_Animal(Entity_earth_creatures_Animal &&) = delete;
  Entity_earth_creatures_Animal(const Entity_earth_creatures_Animal &) = delete;

  void Update() {
    // Binding codes here
    handle.value("DOG", earth::creatures::Animal::DOG);
    handle.value("CAT", earth::creatures::Animal::CAT);
  }
  HandleT handle;
};

struct Entity_earth_creatures_ValueSet {
  using HandleT = pybind11::enum_<earth::creatures::ValueSet>;
  template <class ParentT>
  explicit Entity_earth_creatures_ValueSet(ParentT &&parent_h)
      : handle{pybind11::enum_<earth::creatures::ValueSet>(
            std::forward<ParentT>(parent_h), "ValueSet",
            pybind11::arithmetic())} {}

  Entity_earth_creatures_ValueSet(Entity_earth_creatures_ValueSet &&) = delete;
  Entity_earth_creatures_ValueSet(const Entity_earth_creatures_ValueSet &) =
      delete;

  void Update() {
    // Binding codes here
    handle.value("LOW", earth::creatures::ValueSet::LOW);
    handle.value("MIDDLE", earth::creatures::ValueSet::MIDDLE);
    handle.value("HIGH", earth::creatures::ValueSet::HIGH);
  }
  HandleT handle;
};

struct Entity_earth_creatures_Home {
  using HandleT = pybind11::class_<earth::creatures::Home>;
  template <class ParentT>
  explicit Entity_earth_creatures_Home(ParentT &&parent_h)
      : handle{pybind11::class_<earth::creatures::Home>(
            std::forward<ParentT>(parent_h), "Home")} {}

  Entity_earth_creatures_Home(Entity_earth_creatures_Home &&) = delete;
  Entity_earth_creatures_Home(const Entity_earth_creatures_Home &) = delete;

  void Update() {
    // Binding codes here
    handle.def(pybind11::init<>());
    handle.def(
        "Method",
        static_cast<double (earth::creatures::Home::*)(std::string &, int)>(
            &earth::creatures::Home::Method));
  }
  HandleT handle;
};

struct Entity_earth_creatures_Home_Food {
  using HandleT = pybind11::enum_<earth::creatures::Home::Food>;
  template <class ParentT>
  explicit Entity_earth_creatures_Home_Food(ParentT &&parent_h)
      : handle{pybind11::enum_<earth::creatures::Home::Food>(
            std::forward<ParentT>(parent_h), "Food", pybind11::arithmetic())} {}

  Entity_earth_creatures_Home_Food(Entity_earth_creatures_Home_Food &&) =
      delete;
  Entity_earth_creatures_Home_Food(const Entity_earth_creatures_Home_Food &) =
      delete;

  void Update() {
    // Binding codes here
    handle.value("MEAT", earth::creatures::Home::Food::MEAT);
    handle.value("RICE", earth::creatures::Home::Food::RICE);
  }
  HandleT handle;
};

struct Entity_earth_creatures_SweetHome {
  using HandleT = pybind11::class_<earth::creatures::SweetHome>;
  template <class ParentT>
  explicit Entity_earth_creatures_SweetHome(ParentT &&parent_h)
      : handle{pybind11::class_<earth::creatures::SweetHome>(
            std::forward<ParentT>(parent_h), "SweetHome")} {}

  Entity_earth_creatures_SweetHome(Entity_earth_creatures_SweetHome &&) =
      delete;
  Entity_earth_creatures_SweetHome(const Entity_earth_creatures_SweetHome &) =
      delete;

  void Update() {
    // Binding codes here
    handle.def(
        pybind11::init<int, float, const std::string &, std::string *>());
    handle.def("Method",
               static_cast<void (earth::creatures::SweetHome::*)(int)>(
                   &earth::creatures::SweetHome::Method));
    handle.def(
        "Method",
        static_cast<void (earth::creatures::SweetHome::*)(std::string &)>(
            &earth::creatures::SweetHome::Method));
    handle.def("VirtualMethod",
               static_cast<void (earth::creatures::SweetHome::*)(int)>(
                   &earth::creatures::SweetHome::VirtualMethod));
    handle.def(
        "VirtualMethod",
        static_cast<void (earth::creatures::SweetHome::*)(std::string &)>(
            &earth::creatures::SweetHome::VirtualMethod));
    handle.def_static(
        "StaticMethod",
        static_cast<void (*)(int)>(&earth::creatures::SweetHome::StaticMethod));
    handle.def_static("StaticMethod",
                      static_cast<void (*)(std::string &)>(
                          &earth::creatures::SweetHome::StaticMethod));
    handle.def_readwrite("member", &earth::creatures::SweetHome::member);
  }
  HandleT handle;
};

/**
 * Create all entities, return a callable guard that can be called to update all
 *entities. If the returned guard is not called, the guard will call the update
 *function on its destruction.
 **/
[[nodiscard]] pybind11_weaver::CallUpdateGuard DeclFn(pybind11::module &m) {
  auto v0 = std::make_shared<Entity_TopFunction>(m);
  auto v1 = std::make_shared<Entity_TopFunctionDef>(m);
  auto v2 = std::make_shared<Entity_earth>(m);
  auto v3 = std::make_shared<Entity_earth_creatures>(v2->handle);
  auto v4 = std::make_shared<Entity_earth_creatures_NSFunction>(v3->handle);
  auto v5 = std::make_shared<Entity_earth_creatures_NSFunctionDef>(v3->handle);
  auto v6 = std::make_shared<Entity_earth_creatures_Animal>(v3->handle);
  auto v7 = std::make_shared<Entity_earth_creatures_ValueSet>(v3->handle);
  auto v8 = std::make_shared<Entity_earth_creatures_Home>(v3->handle);
  auto v9 = std::make_shared<Entity_earth_creatures_Home_Food>(v8->handle);
  auto v10 = std::make_shared<Entity_earth_creatures_SweetHome>(v3->handle);

  auto update_fn = [=]() {
    v0->Update();
    v1->Update();
    v2->Update();
    v3->Update();
    v4->Update();
    v5->Update();
    v6->Update();
    v7->Update();
    v8->Update();
    v9->Update();
    v10->Update();
  };
  return {update_fn};
}

} // anonymous namespace
