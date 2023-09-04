
// GENERATED AT 09/04/2023, 11:33:51

#include "sample.h"

#ifndef GITHUB_COM_PYBIND11_WEAVER
#define GITHUB_COM_PYBIND11_WEAVER

#include <pybind11/pybind11.h>

namespace pybind11_weaver {

class CallUpdateGuard {
public:
  using Fn = std::function<void(void)>;
  CallUpdateGuard(Fn fn) : fn_(fn) {}

  CallUpdateGuard(CallUpdateGuard &&rhs) {
    this->fn_ = rhs.fn_;
    rhs.fn_ = nullptr;
  }

  void operator()() {
    if (fn_) {
      fn_();
      fn_ = nullptr;
    }
  }

  ~CallUpdateGuard() { this->operator()(); }

private:
  Fn fn_;
};

struct EntityBase {
  virtual ~EntityBase() = default;
};

struct ParentEntity {
  explicit ParentEntity(pybind11::module_ &parent_h) : module_{&parent_h} {}
  explicit ParentEntity(pybind11::detail::generic_type &parent_h)
      : type_{&parent_h} {}
  explicit operator pybind11::module_ &() { return *module_; }
  explicit operator pybind11::detail::generic_type &() { return *type_; }
  operator pybind11::handle &() {
    if (module_) {
      return *module_;
    } else {
      return *type_;
    }
  }

private:
  pybind11::detail::generic_type *type_ = nullptr;
  pybind11::module_ *module_ = nullptr;
};

} // namespace pybind11_weaver
#endif // GITHUB_COM_PYBIND11_WEAVER

namespace {

using pybind11_weaver::EntityBase;
using pybind11_weaver::ParentEntity;

struct Entity_TopFunction : public EntityBase {
  using HandleT = pybind11::module_ &;
  explicit Entity_TopFunction(ParentEntity &&parent_h)
      : handle{static_cast<pybind11::module_ &>(parent_h)} {}

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

struct Entity_TopFunctionDef : public EntityBase {
  using HandleT = pybind11::module_ &;
  explicit Entity_TopFunctionDef(ParentEntity &&parent_h)
      : handle{static_cast<pybind11::module_ &>(parent_h)} {}

  Entity_TopFunctionDef(Entity_TopFunctionDef &&) = delete;
  Entity_TopFunctionDef(const Entity_TopFunctionDef &) = delete;

  void Update() {
    // Binding codes here
    handle.def("TopFunctionDef", static_cast<void (*)()>(&TopFunctionDef));
  }
  HandleT handle;
};

struct Entity_earth : public EntityBase {
  using HandleT = pybind11::module_;
  explicit Entity_earth(ParentEntity &&parent_h)
      : handle{static_cast<pybind11::module_ &>(parent_h).def_submodule(
            "earth")} {}

  Entity_earth(Entity_earth &&) = delete;
  Entity_earth(const Entity_earth &) = delete;

  void Update() {
    // Binding codes here
  }
  HandleT handle;
};

struct Entity_earth_creatures : public EntityBase {
  using HandleT = pybind11::module_;
  explicit Entity_earth_creatures(ParentEntity &&parent_h)
      : handle{static_cast<pybind11::module_ &>(parent_h).def_submodule(
            "creatures")} {}

  Entity_earth_creatures(Entity_earth_creatures &&) = delete;
  Entity_earth_creatures(const Entity_earth_creatures &) = delete;

  void Update() {
    // Binding codes here
  }
  HandleT handle;
};

struct Entity_earth_creatures_NSFunction : public EntityBase {
  using HandleT = pybind11::module_ &;
  explicit Entity_earth_creatures_NSFunction(ParentEntity &&parent_h)
      : handle{static_cast<pybind11::module_ &>(parent_h)} {}

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

struct Entity_earth_creatures_NSFunctionDef : public EntityBase {
  using HandleT = pybind11::module_ &;
  explicit Entity_earth_creatures_NSFunctionDef(ParentEntity &&parent_h)
      : handle{static_cast<pybind11::module_ &>(parent_h)} {}

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

struct Entity_earth_creatures_Animal : public EntityBase {
  using HandleT = pybind11::enum_<earth::creatures::Animal>;
  explicit Entity_earth_creatures_Animal(ParentEntity &&parent_h)
      : handle{pybind11::enum_<earth::creatures::Animal>(
            parent_h, "Animal", pybind11::arithmetic())} {}

  Entity_earth_creatures_Animal(Entity_earth_creatures_Animal &&) = delete;
  Entity_earth_creatures_Animal(const Entity_earth_creatures_Animal &) = delete;

  void Update() {
    // Binding codes here
    handle.value("DOG", earth::creatures::Animal::DOG);
    handle.value("CAT", earth::creatures::Animal::CAT);
  }
  HandleT handle;
};

struct Entity_earth_creatures_ValueSet : public EntityBase {
  using HandleT = pybind11::enum_<earth::creatures::ValueSet>;
  explicit Entity_earth_creatures_ValueSet(ParentEntity &&parent_h)
      : handle{pybind11::enum_<earth::creatures::ValueSet>(
            parent_h, "ValueSet", pybind11::arithmetic())} {}

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

struct Entity_earth_creatures_Home : public EntityBase {
  using HandleT = pybind11::class_<earth::creatures::Home>;
  explicit Entity_earth_creatures_Home(ParentEntity &&parent_h)
      : handle{pybind11::class_<earth::creatures::Home>(parent_h, "Home")} {}

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

struct Entity_earth_creatures_Home_Food : public EntityBase {
  using HandleT = pybind11::enum_<earth::creatures::Home::Food>;
  explicit Entity_earth_creatures_Home_Food(ParentEntity &&parent_h)
      : handle{pybind11::enum_<earth::creatures::Home::Food>(
            parent_h, "Food", pybind11::arithmetic())} {}

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

struct Entity_earth_creatures_SweetHome : public EntityBase {
  using HandleT = pybind11::class_<earth::creatures::SweetHome>;
  explicit Entity_earth_creatures_SweetHome(ParentEntity &&parent_h)
      : handle{pybind11::class_<earth::creatures::SweetHome>(parent_h,
                                                             "SweetHome")} {}

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
  auto v0 = std::make_shared<Entity_TopFunction>(ParentEntity(m));
  auto v1 = std::make_shared<Entity_TopFunctionDef>(ParentEntity(m));
  auto v2 = std::make_shared<Entity_earth>(ParentEntity(m));
  auto v3 = std::make_shared<Entity_earth_creatures>(ParentEntity(v2->handle));
  auto v4 = std::make_shared<Entity_earth_creatures_NSFunction>(
      ParentEntity(v3->handle));
  auto v5 = std::make_shared<Entity_earth_creatures_NSFunctionDef>(
      ParentEntity(v3->handle));
  auto v6 =
      std::make_shared<Entity_earth_creatures_Animal>(ParentEntity(v3->handle));
  auto v7 = std::make_shared<Entity_earth_creatures_ValueSet>(
      ParentEntity(v3->handle));
  auto v8 =
      std::make_shared<Entity_earth_creatures_Home>(ParentEntity(v3->handle));
  auto v9 = std::make_shared<Entity_earth_creatures_Home_Food>(
      ParentEntity(v8->handle));
  auto v10 = std::make_shared<Entity_earth_creatures_SweetHome>(
      ParentEntity(v3->handle));

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
