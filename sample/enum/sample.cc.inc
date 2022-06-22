
// GENERATED AT 06/22/2022, 17:46:06

#include "sample.h"

#include <pybind11_weaver/pybind11_weaver.h>

namespace {

struct Entity_earth {
  using HandleT = pybind11::module_; // User are free to modify HandleT
  template<class ParentT>
  explicit Entity_earth(ParentT &&parent_h):handle{std::forward<ParentT>(parent_h).def_submodule("earth")} {
  }

  Entity_earth(Entity_earth &&) = delete;
  Entity_earth(const Entity_earth &) = delete;

  void Update() {
    //Binding codes here

  }
  HandleT handle;
  [[maybe_unused]] const char *qualified_name = "earth"; // User MUST NOT modify this decl
};

struct Entity_earth_creatures {
  using HandleT = pybind11::module_; // User are free to modify HandleT
  template<class ParentT>
  explicit Entity_earth_creatures(ParentT &&parent_h):handle{
      std::forward<ParentT>(parent_h).def_submodule("creatures")} {
  }

  Entity_earth_creatures(Entity_earth_creatures &&) = delete;
  Entity_earth_creatures(const Entity_earth_creatures &) = delete;

  void Update() {
    //Binding codes here

  }
  HandleT handle;
  [[maybe_unused]] const char *qualified_name = "earth::creatures"; // User MUST NOT modify this decl
};

struct Entity_earth_creatures_Animal {
  using HandleT = pybind11::enum_<earth::creatures::Animal>; // User are free to modify HandleT
  template<class ParentT>
  explicit Entity_earth_creatures_Animal(ParentT &&parent_h):handle{
      pybind11::enum_<earth::creatures::Animal>(std::forward<ParentT>(parent_h), "Animal", pybind11::arithmetic())} {
  }

  Entity_earth_creatures_Animal(Entity_earth_creatures_Animal &&) = delete;
  Entity_earth_creatures_Animal(const Entity_earth_creatures_Animal &) = delete;

  void Update() {
    //Binding codes here
    handle.value("DOG", earth::creatures::Animal::DOG);
    handle.value("CAT", earth::creatures::Animal::CAT);
  }
  HandleT handle;
  [[maybe_unused]] const char *qualified_name = "earth::creatures::Animal"; // User MUST NOT modify this decl
};

struct Entity_earth_creatures_Plant {
  using HandleT = pybind11::enum_<earth::creatures::Plant>; // User are free to modify HandleT
  template<class ParentT>
  explicit Entity_earth_creatures_Plant(ParentT &&parent_h):handle{
      pybind11::enum_<earth::creatures::Plant>(std::forward<ParentT>(parent_h), "Plant", pybind11::arithmetic())} {
  }

  Entity_earth_creatures_Plant(Entity_earth_creatures_Plant &&) = delete;
  Entity_earth_creatures_Plant(const Entity_earth_creatures_Plant &) = delete;

  void Update() {
    //Binding codes here
    handle.value("TREE", earth::creatures::Plant::TREE);
    handle.value("FLOWER", earth::creatures::Plant::FLOWER);
  }
  HandleT handle;
  [[maybe_unused]] const char *qualified_name = "earth::creatures::Plant"; // User MUST NOT modify this decl
};

struct Entity_earth_creatures_ValueSet {
  using HandleT = pybind11::enum_<earth::creatures::ValueSet>; // User are free to modify HandleT
  template<class ParentT>
  explicit Entity_earth_creatures_ValueSet(ParentT &&parent_h):handle{
      pybind11::enum_<earth::creatures::ValueSet>(std::forward<ParentT>(parent_h),
                                                  "ValueSet",
                                                  pybind11::arithmetic())} {
  }

  Entity_earth_creatures_ValueSet(Entity_earth_creatures_ValueSet &&) = delete;
  Entity_earth_creatures_ValueSet(const Entity_earth_creatures_ValueSet &) = delete;

  void Update() {
    //Binding codes here
    handle.value("LOW", earth::creatures::ValueSet::LOW);
    handle.value("MIDDLE", earth::creatures::ValueSet::MIDDLE);
    handle.value("HIGH", earth::creatures::ValueSet::HIGH);
  }
  HandleT handle;
  [[maybe_unused]] const char *qualified_name = "earth::creatures::ValueSet"; // User MUST NOT modify this decl
};

struct Entity_earth_creatures_Home {
  using HandleT = pybind11::class_<earth::creatures::Home>; // User are free to modify HandleT
  template<class ParentT>
  explicit Entity_earth_creatures_Home(ParentT &&parent_h):handle{
      pybind11::class_<earth::creatures::Home>(std::forward<ParentT>(parent_h), "Home")} {
  }

  Entity_earth_creatures_Home(Entity_earth_creatures_Home &&) = delete;
  Entity_earth_creatures_Home(const Entity_earth_creatures_Home &) = delete;

  void Update() {
    //Binding codes here

  }
  HandleT handle;
  [[maybe_unused]] const char *qualified_name = "earth::creatures::Home"; // User MUST NOT modify this decl
};

struct Entity_earth_creatures_Home_Tool {
  using HandleT = pybind11::enum_<earth::creatures::Home::Tool>; // User are free to modify HandleT
  template<class ParentT>
  explicit Entity_earth_creatures_Home_Tool(ParentT &&parent_h):handle{
      pybind11::enum_<earth::creatures::Home::Tool>(std::forward<ParentT>(parent_h), "Tool", pybind11::arithmetic())} {
  }

  Entity_earth_creatures_Home_Tool(Entity_earth_creatures_Home_Tool &&) = delete;
  Entity_earth_creatures_Home_Tool(const Entity_earth_creatures_Home_Tool &) = delete;

  void Update() {
    //Binding codes here
    handle.value("PAN", earth::creatures::Home::Tool::PAN);
    handle.value("ROPE", earth::creatures::Home::Tool::ROPE);
  }
  HandleT handle;
  [[maybe_unused]] const char *qualified_name = "earth::creatures::Home::Tool"; // User MUST NOT modify this decl
};

struct Entity_earth_creatures_Home_Food {
  using HandleT = pybind11::enum_<earth::creatures::Home::Food>; // User are free to modify HandleT
  template<class ParentT>
  explicit Entity_earth_creatures_Home_Food(ParentT &&parent_h):handle{
      pybind11::enum_<earth::creatures::Home::Food>(std::forward<ParentT>(parent_h), "Food", pybind11::arithmetic())} {
  }

  Entity_earth_creatures_Home_Food(Entity_earth_creatures_Home_Food &&) = delete;
  Entity_earth_creatures_Home_Food(const Entity_earth_creatures_Home_Food &) = delete;

  void Update() {
    //Binding codes here
    handle.value("MEAT", earth::creatures::Home::Food::MEAT);
    handle.value("RICE", earth::creatures::Home::Food::RICE);
  }
  HandleT handle;
  [[maybe_unused]] const char *qualified_name = "earth::creatures::Home::Food"; // User MUST NOT modify this decl
};

/**
* Create all entities, return a callable guard that can be called to update all entities.
* If the returned guard is not called, the guard will call the update function on its destruction.
**/
[[nodiscard]] pybind11_weaver::CallUpdateGuard DeclFn(pybind11::module &m) {
  auto v0 = std::make_shared<Entity_earth>(m);
  auto v1 = std::make_shared<Entity_earth_creatures>(v0->handle);
  auto v2 = std::make_shared<Entity_earth_creatures_Animal>(v1->handle);
  auto v3 = std::make_shared<Entity_earth_creatures_Plant>(v1->handle);
  auto v4 = std::make_shared<Entity_earth_creatures_ValueSet>(v1->handle);
  auto v5 = std::make_shared<Entity_earth_creatures_Home>(v1->handle);
  auto v6 = std::make_shared<Entity_earth_creatures_Home_Tool>(v5->handle);
  auto v7 = std::make_shared<Entity_earth_creatures_Home_Food>(v5->handle);

  auto update_fn = [=]() {
    v0->Update();
    v1->Update();
    v2->Update();
    v3->Update();
    v4->Update();
    v5->Update();
    v6->Update();
    v7->Update();
  };
  return {update_fn};
}

} // anonymous namespace

