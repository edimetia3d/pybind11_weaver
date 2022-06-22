
// GENERATED AT 06/22/2022, 15:24:13

#include "sample.h"

#include <pybind11_weaver/pybind11_weaver.h>

namespace {

struct Entity0 {
  using HandleT = pybind11::module_; // User are free to modify HandleT
  template<class ParentT>
  explicit Entity0(ParentT &&parent_h):handle{std::forward<ParentT>(parent_h).def_submodule("earth")} {
  }

  Entity0(Entity0 &&) = delete;
  Entity0(const Entity0 &) = delete;

  void Update() {
    //Binding codes here

  }
  HandleT handle;
  [[maybe_unused]] const char *qualified_name = "earth"; // User MUST NOT modify this decl
};

struct Entity1 {
  using HandleT = pybind11::module_; // User are free to modify HandleT
  template<class ParentT>
  explicit Entity1(ParentT &&parent_h):handle{std::forward<ParentT>(parent_h).def_submodule("creatures")} {
  }

  Entity1(Entity1 &&) = delete;
  Entity1(const Entity1 &) = delete;

  void Update() {
    //Binding codes here

  }
  HandleT handle;
  [[maybe_unused]] const char *qualified_name = "earth::creatures"; // User MUST NOT modify this decl
};

struct Entity2 {
  using HandleT = pybind11::enum_<earth::creatures::Animal>; // User are free to modify HandleT
  template<class ParentT>
  explicit Entity2(ParentT &&parent_h):handle{
      pybind11::enum_<earth::creatures::Animal>(std::forward<ParentT>(parent_h), "Animal", pybind11::arithmetic())} {
  }

  Entity2(Entity2 &&) = delete;
  Entity2(const Entity2 &) = delete;

  void Update() {
    //Binding codes here
    handle.value("DOG", earth::creatures::Animal::DOG);
    handle.value("CAT", earth::creatures::Animal::CAT);
  }
  HandleT handle;
  [[maybe_unused]] const char *qualified_name = "earth::creatures::Animal"; // User MUST NOT modify this decl
};

struct Entity3 {
  using HandleT = pybind11::enum_<earth::creatures::Plant>; // User are free to modify HandleT
  template<class ParentT>
  explicit Entity3(ParentT &&parent_h):handle{
      pybind11::enum_<earth::creatures::Plant>(std::forward<ParentT>(parent_h), "Plant", pybind11::arithmetic())} {
  }

  Entity3(Entity3 &&) = delete;
  Entity3(const Entity3 &) = delete;

  void Update() {
    //Binding codes here
    handle.value("TREE", earth::creatures::Plant::TREE);
    handle.value("FLOWER", earth::creatures::Plant::FLOWER);
  }
  HandleT handle;
  [[maybe_unused]] const char *qualified_name = "earth::creatures::Plant"; // User MUST NOT modify this decl
};

struct Entity4 {
  using HandleT = pybind11::enum_<earth::creatures::ValueSet>; // User are free to modify HandleT
  template<class ParentT>
  explicit Entity4(ParentT &&parent_h):handle{
      pybind11::enum_<earth::creatures::ValueSet>(std::forward<ParentT>(parent_h),
                                                  "ValueSet",
                                                  pybind11::arithmetic())} {
  }

  Entity4(Entity4 &&) = delete;
  Entity4(const Entity4 &) = delete;

  void Update() {
    //Binding codes here
    handle.value("LOW", earth::creatures::ValueSet::LOW);
    handle.value("MIDDLE", earth::creatures::ValueSet::MIDDLE);
    handle.value("HIGH", earth::creatures::ValueSet::HIGH);
  }
  HandleT handle;
  [[maybe_unused]] const char *qualified_name = "earth::creatures::ValueSet"; // User MUST NOT modify this decl
};

struct Entity5 {
  using HandleT = pybind11::class_<earth::creatures::Home>; // User are free to modify HandleT
  template<class ParentT>
  explicit Entity5(ParentT &&parent_h):handle{
      pybind11::class_<earth::creatures::Home>(std::forward<ParentT>(parent_h), "Home")} {
  }

  Entity5(Entity5 &&) = delete;
  Entity5(const Entity5 &) = delete;

  void Update() {
    //Binding codes here

  }
  HandleT handle;
  [[maybe_unused]] const char *qualified_name = "earth::creatures::Home"; // User MUST NOT modify this decl
};

struct Entity6 {
  using HandleT = pybind11::enum_<earth::creatures::Home::Tool>; // User are free to modify HandleT
  template<class ParentT>
  explicit Entity6(ParentT &&parent_h):handle{
      pybind11::enum_<earth::creatures::Home::Tool>(std::forward<ParentT>(parent_h), "Tool", pybind11::arithmetic())} {
  }

  Entity6(Entity6 &&) = delete;
  Entity6(const Entity6 &) = delete;

  void Update() {
    //Binding codes here
    handle.value("PAN", earth::creatures::Home::Tool::PAN);
    handle.value("ROPE", earth::creatures::Home::Tool::ROPE);
  }
  HandleT handle;
  [[maybe_unused]] const char *qualified_name = "earth::creatures::Home::Tool"; // User MUST NOT modify this decl
};

struct Entity7 {
  using HandleT = pybind11::enum_<earth::creatures::Home::Food>; // User are free to modify HandleT
  template<class ParentT>
  explicit Entity7(ParentT &&parent_h):handle{
      pybind11::enum_<earth::creatures::Home::Food>(std::forward<ParentT>(parent_h), "Food", pybind11::arithmetic())} {
  }

  Entity7(Entity7 &&) = delete;
  Entity7(const Entity7 &) = delete;

  void Update() {
    //Binding codes here
    handle.value("MEAT", earth::creatures::Home::Food::MEAT);
    handle.value("RICE", earth::creatures::Home::Food::RICE);
  }
  HandleT handle;
  [[maybe_unused]] const char *qualified_name = "earth::creatures::Home::Food"; // User MUST NOT modify this decl
};

pybind11_weaver::CallUpdateGuard DeclEnums(pybind11::module &m) {
  auto v0_earth = std::make_shared<Entity0>(m);
  auto v1_creatures = std::make_shared<Entity1>(v0_earth->handle);
  auto v2_Animal = std::make_shared<Entity2>(v1_creatures->handle);
  auto v3_Plant = std::make_shared<Entity3>(v1_creatures->handle);
  auto v4_ValueSet = std::make_shared<Entity4>(v1_creatures->handle);
  auto v5_Home = std::make_shared<Entity5>(v1_creatures->handle);
  auto v6_Tool = std::make_shared<Entity6>(v5_Home->handle);
  auto v7_Food = std::make_shared<Entity7>(v5_Home->handle);

  auto update_fn = [=]() {
    v0_earth->Update();
    v1_creatures->Update();
    v2_Animal->Update();
    v3_Plant->Update();
    v4_ValueSet->Update();
    v5_Home->Update();
    v6_Tool->Update();
    v7_Food->Update();
  };
  return {update_fn};
}

} // anonymous namespace

