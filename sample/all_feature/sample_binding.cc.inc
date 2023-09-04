
// GENERATED AT 09/04/2023, 16:20:00

#include "sample.h"

#ifndef GITHUB_COM_PYBIND11_WEAVER
#define GITHUB_COM_PYBIND11_WEAVER
#include <functional>
#include <map>

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

struct EntityScope {
  explicit EntityScope(pybind11::module_ &parent_h) : module_{&parent_h} {}
  explicit EntityScope(pybind11::detail::generic_type &parent_h)
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

struct EntityBase {
  virtual ~EntityBase() = default;

  virtual void Update() = 0;

  virtual EntityScope AsScope() = 0;
};

using RegistryT =
    std::map<std::string,
             std::function<std::shared_ptr<EntityBase>(EntityScope &&)>>;

template <class EntityT>
std::shared_ptr<EntityBase> CreateEntity(EntityScope &&parent_h,
                                         const RegistryT &registry) {
  auto key = std::string(EntityT::Key());
  if (registry.count(key) == 0) {
    return std::make_shared<EntityT>(std::move(parent_h));
  } else {
    auto fn = registry.at(key);
    return fn(std::move(parent_h));
  }
}

} // namespace pybind11_weaver
#endif // GITHUB_COM_PYBIND11_WEAVER

namespace {

using pybind11_weaver::EntityBase;
using pybind11_weaver::EntityScope;

template <class Pybind11T> struct Bind_TopFunction : public EntityBase {

  explicit Bind_TopFunction(EntityScope parent_h) {}

  virtual void Bind(void *handle_) {
    auto &handle = *reinterpret_cast<Pybind11T *>(handle_);
    handle.def("TopFunction", static_cast<void (*)(int)>(&TopFunction));
    handle.def("TopFunction",
               static_cast<void (*)(std::string &)>(&TopFunction));
  }

  static const char *Key() { return "TopFunction"; }
};

struct Entity_TopFunction
    : public Bind_TopFunction<std::decay_t<pybind11::module_ &>> {

  explicit Entity_TopFunction(EntityScope parent_h)
      : Bind_TopFunction<std::decay_t<pybind11::module_ &>>(parent_h),
        handle{static_cast<pybind11::module_ &>(parent_h)} {}

  void Update() override { Bind(&handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::module_ &handle;
};

template <class Pybind11T> struct Bind_TopFunctionDef : public EntityBase {

  explicit Bind_TopFunctionDef(EntityScope parent_h) {}

  virtual void Bind(void *handle_) {
    auto &handle = *reinterpret_cast<Pybind11T *>(handle_);
    handle.def("TopFunction", static_cast<void (*)(int)>(&TopFunction));
    handle.def("TopFunction",
               static_cast<void (*)(std::string &)>(&TopFunction));
  }

  static const char *Key() { return "TopFunctionDef"; }
};

struct Entity_TopFunctionDef
    : public Bind_TopFunctionDef<std::decay_t<pybind11::module_ &>> {

  explicit Entity_TopFunctionDef(EntityScope parent_h)
      : Bind_TopFunctionDef<std::decay_t<pybind11::module_ &>>(parent_h),
        handle{static_cast<pybind11::module_ &>(parent_h)} {}

  void Update() override { Bind(&handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::module_ &handle;
};

template <class Pybind11T> struct Bind_earth : public EntityBase {

  explicit Bind_earth(EntityScope parent_h) {}

  virtual void Bind(void *handle_) {
    auto &handle = *reinterpret_cast<Pybind11T *>(handle_);
    handle.def("TopFunction", static_cast<void (*)(int)>(&TopFunction));
    handle.def("TopFunction",
               static_cast<void (*)(std::string &)>(&TopFunction));
  }

  static const char *Key() { return "earth"; }
};

struct Entity_earth : public Bind_earth<std::decay_t<pybind11::module_>> {

  explicit Entity_earth(EntityScope parent_h)
      : Bind_earth<std::decay_t<pybind11::module_>>(parent_h),
        handle{
            static_cast<pybind11::module_ &>(parent_h).def_submodule("earth")} {
  }

  void Update() override { Bind(&handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::module_ handle;
};

template <class Pybind11T> struct Bind_earth_creatures : public EntityBase {

  explicit Bind_earth_creatures(EntityScope parent_h) {}

  virtual void Bind(void *handle_) {
    auto &handle = *reinterpret_cast<Pybind11T *>(handle_);
    handle.def("TopFunction", static_cast<void (*)(int)>(&TopFunction));
    handle.def("TopFunction",
               static_cast<void (*)(std::string &)>(&TopFunction));
  }

  static const char *Key() { return "earth::creatures"; }
};

struct Entity_earth_creatures
    : public Bind_earth_creatures<std::decay_t<pybind11::module_>> {

  explicit Entity_earth_creatures(EntityScope parent_h)
      : Bind_earth_creatures<std::decay_t<pybind11::module_>>(parent_h),
        handle{static_cast<pybind11::module_ &>(parent_h).def_submodule(
            "creatures")} {}

  void Update() override { Bind(&handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::module_ handle;
};

template <class Pybind11T>
struct Bind_earth_creatures_NSFunction : public EntityBase {

  explicit Bind_earth_creatures_NSFunction(EntityScope parent_h) {}

  virtual void Bind(void *handle_) {
    auto &handle = *reinterpret_cast<Pybind11T *>(handle_);
    handle.def("TopFunction", static_cast<void (*)(int)>(&TopFunction));
    handle.def("TopFunction",
               static_cast<void (*)(std::string &)>(&TopFunction));
  }

  static const char *Key() { return "earth::creatures::NSFunction"; }
};

struct Entity_earth_creatures_NSFunction
    : public Bind_earth_creatures_NSFunction<
          std::decay_t<pybind11::module_ &>> {

  explicit Entity_earth_creatures_NSFunction(EntityScope parent_h)
      : Bind_earth_creatures_NSFunction<std::decay_t<pybind11::module_ &>>(
            parent_h),
        handle{static_cast<pybind11::module_ &>(parent_h)} {}

  void Update() override { Bind(&handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::module_ &handle;
};

template <class Pybind11T>
struct Bind_earth_creatures_NSFunctionDef : public EntityBase {

  explicit Bind_earth_creatures_NSFunctionDef(EntityScope parent_h) {}

  virtual void Bind(void *handle_) {
    auto &handle = *reinterpret_cast<Pybind11T *>(handle_);
    handle.def("TopFunction", static_cast<void (*)(int)>(&TopFunction));
    handle.def("TopFunction",
               static_cast<void (*)(std::string &)>(&TopFunction));
  }

  static const char *Key() { return "earth::creatures::NSFunctionDef"; }
};

struct Entity_earth_creatures_NSFunctionDef
    : public Bind_earth_creatures_NSFunctionDef<
          std::decay_t<pybind11::module_ &>> {

  explicit Entity_earth_creatures_NSFunctionDef(EntityScope parent_h)
      : Bind_earth_creatures_NSFunctionDef<std::decay_t<pybind11::module_ &>>(
            parent_h),
        handle{static_cast<pybind11::module_ &>(parent_h)} {}

  void Update() override { Bind(&handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::module_ &handle;
};

template <class Pybind11T>
struct Bind_earth_creatures_Animal : public EntityBase {

  explicit Bind_earth_creatures_Animal(EntityScope parent_h) {}

  virtual void Bind(void *handle_) {
    auto &handle = *reinterpret_cast<Pybind11T *>(handle_);
    handle.def("TopFunction", static_cast<void (*)(int)>(&TopFunction));
    handle.def("TopFunction",
               static_cast<void (*)(std::string &)>(&TopFunction));
  }

  static const char *Key() { return "earth::creatures::Animal"; }
};

struct Entity_earth_creatures_Animal
    : public Bind_earth_creatures_Animal<
          std::decay_t<pybind11::enum_<earth::creatures::Animal>>> {

  explicit Entity_earth_creatures_Animal(EntityScope parent_h)
      : Bind_earth_creatures_Animal<
            std::decay_t<pybind11::enum_<earth::creatures::Animal>>>(parent_h),
        handle{parent_h, "Animal", pybind11::arithmetic()} {}

  void Update() override { Bind(&handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::enum_<earth::creatures::Animal> handle;
};

template <class Pybind11T>
struct Bind_earth_creatures_ValueSet : public EntityBase {

  explicit Bind_earth_creatures_ValueSet(EntityScope parent_h) {}

  virtual void Bind(void *handle_) {
    auto &handle = *reinterpret_cast<Pybind11T *>(handle_);
    handle.def("TopFunction", static_cast<void (*)(int)>(&TopFunction));
    handle.def("TopFunction",
               static_cast<void (*)(std::string &)>(&TopFunction));
  }

  static const char *Key() { return "earth::creatures::ValueSet"; }
};

struct Entity_earth_creatures_ValueSet
    : public Bind_earth_creatures_ValueSet<
          std::decay_t<pybind11::enum_<earth::creatures::ValueSet>>> {

  explicit Entity_earth_creatures_ValueSet(EntityScope parent_h)
      : Bind_earth_creatures_ValueSet<
            std::decay_t<pybind11::enum_<earth::creatures::ValueSet>>>(
            parent_h),
        handle{parent_h, "ValueSet", pybind11::arithmetic()} {}

  void Update() override { Bind(&handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::enum_<earth::creatures::ValueSet> handle;
};

template <class Pybind11T>
struct Bind_earth_creatures_Home : public EntityBase {

  explicit Bind_earth_creatures_Home(EntityScope parent_h) {}

  virtual void Bind(void *handle_) {
    auto &handle = *reinterpret_cast<Pybind11T *>(handle_);
    handle.def("TopFunction", static_cast<void (*)(int)>(&TopFunction));
    handle.def("TopFunction",
               static_cast<void (*)(std::string &)>(&TopFunction));
  }

  static const char *Key() { return "earth::creatures::Home"; }
};

struct Entity_earth_creatures_Home
    : public Bind_earth_creatures_Home<
          std::decay_t<pybind11::class_<earth::creatures::Home>>> {

  explicit Entity_earth_creatures_Home(EntityScope parent_h)
      : Bind_earth_creatures_Home<
            std::decay_t<pybind11::class_<earth::creatures::Home>>>(parent_h),
        handle{parent_h, "Home"} {}

  void Update() override { Bind(&handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::class_<earth::creatures::Home> handle;
};

template <class Pybind11T>
struct Bind_earth_creatures_Home_Food : public EntityBase {

  explicit Bind_earth_creatures_Home_Food(EntityScope parent_h) {}

  virtual void Bind(void *handle_) {
    auto &handle = *reinterpret_cast<Pybind11T *>(handle_);
    handle.def("TopFunction", static_cast<void (*)(int)>(&TopFunction));
    handle.def("TopFunction",
               static_cast<void (*)(std::string &)>(&TopFunction));
  }

  static const char *Key() { return "earth::creatures::Home::Food"; }
};

struct Entity_earth_creatures_Home_Food
    : public Bind_earth_creatures_Home_Food<
          std::decay_t<pybind11::enum_<earth::creatures::Home::Food>>> {

  explicit Entity_earth_creatures_Home_Food(EntityScope parent_h)
      : Bind_earth_creatures_Home_Food<
            std::decay_t<pybind11::enum_<earth::creatures::Home::Food>>>(
            parent_h),
        handle{parent_h, "Food", pybind11::arithmetic()} {}

  void Update() override { Bind(&handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::enum_<earth::creatures::Home::Food> handle;
};

template <class Pybind11T>
struct Bind_earth_creatures_SweetHome : public EntityBase {

  explicit Bind_earth_creatures_SweetHome(EntityScope parent_h) {}

  virtual void Bind(void *handle_) {
    auto &handle = *reinterpret_cast<Pybind11T *>(handle_);
    handle.def("TopFunction", static_cast<void (*)(int)>(&TopFunction));
    handle.def("TopFunction",
               static_cast<void (*)(std::string &)>(&TopFunction));
  }

  static const char *Key() { return "earth::creatures::SweetHome"; }
};

struct Entity_earth_creatures_SweetHome
    : public Bind_earth_creatures_SweetHome<
          std::decay_t<pybind11::class_<earth::creatures::SweetHome>>> {

  explicit Entity_earth_creatures_SweetHome(EntityScope parent_h)
      : Bind_earth_creatures_SweetHome<
            std::decay_t<pybind11::class_<earth::creatures::SweetHome>>>(
            parent_h),
        handle{parent_h, "SweetHome"} {}

  void Update() override { Bind(&handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::class_<earth::creatures::SweetHome> handle;
};

/**
 * Create all entities, return a callable guard that can be called to update all
 *entities. If the returned guard is not called, the guard will call the update
 *function on its destruction.
 **/
[[nodiscard]] pybind11_weaver::CallUpdateGuard
DeclFn(pybind11::module &m, const pybind11_weaver::RegistryT &registry) {
  auto v0 = pybind11_weaver::CreateEntity<Entity_TopFunction>(EntityScope(m),
                                                              registry);
  auto v1 = pybind11_weaver::CreateEntity<Entity_TopFunctionDef>(EntityScope(m),
                                                                 registry);
  auto v2 =
      pybind11_weaver::CreateEntity<Entity_earth>(EntityScope(m), registry);
  auto v3 = pybind11_weaver::CreateEntity<Entity_earth_creatures>(v2->AsScope(),
                                                                  registry);
  auto v4 = pybind11_weaver::CreateEntity<Entity_earth_creatures_NSFunction>(
      v3->AsScope(), registry);
  auto v5 = pybind11_weaver::CreateEntity<Entity_earth_creatures_NSFunctionDef>(
      v3->AsScope(), registry);
  auto v6 = pybind11_weaver::CreateEntity<Entity_earth_creatures_Animal>(
      v3->AsScope(), registry);
  auto v7 = pybind11_weaver::CreateEntity<Entity_earth_creatures_ValueSet>(
      v3->AsScope(), registry);
  auto v8 = pybind11_weaver::CreateEntity<Entity_earth_creatures_Home>(
      v3->AsScope(), registry);
  auto v9 = pybind11_weaver::CreateEntity<Entity_earth_creatures_Home_Food>(
      v8->AsScope(), registry);
  auto v10 = pybind11_weaver::CreateEntity<Entity_earth_creatures_SweetHome>(
      v3->AsScope(), registry);

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
