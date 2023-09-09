
// GENERATED AT 09/09/2023, 13:09:27

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
  explicit EntityScope(int64_t, int64_t) {} // a tag for disabled scope
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
  bool IsDisabled() const { return module_ == nullptr && type_ == nullptr; }

private:
  pybind11::detail::generic_type *type_ = nullptr;
  pybind11::module_ *module_ = nullptr;
};

struct EntityBase {
  virtual ~EntityBase() = default;

  virtual void Update() = 0;

  virtual EntityScope AsScope() = 0;
};

struct DisabledEntity : public EntityBase {
  void Update() override {}
  EntityScope AsScope() override { return EntityScope{0, 0}; }
};

struct CustomBindingRegistry {
  using CTorT = std::function<std::shared_ptr<EntityBase>(EntityScope &&)>;
  using RegistryT = std::map<std::string, CTorT>;

  bool contains(const std::string &key) const {
    return registry_.count(key) > 0;
  }
  CTorT at(const std::string &key) const { return registry_.at(key); }

  template <class BindingT> void DisableBinding() {
    auto key = std::string(BindingT::Key());
    registry_.emplace(
        key, [](EntityScope &&) { return std::make_shared<DisabledEntity>(); });
  }

  void RegCustomBinding(const std::string &key, CTorT &&ctor) {
    registry_.emplace(key, std::move(ctor));
  }

  template <class BindingT> void SetCustomBinding() {
    auto key = std::string(BindingT::Key());
    registry_.emplace(key, [](EntityScope &&parent_h) {
      return std::make_shared<BindingT>(std::move(parent_h));
    });
  }

private:
  RegistryT registry_;
};

template <class EntityT>
std::shared_ptr<EntityBase>
CreateEntity(EntityScope &&parent_h, const CustomBindingRegistry &registry) {
  if (parent_h.IsDisabled()) {
    return std::make_shared<DisabledEntity>();
  }
  auto key = std::string(EntityT::Key());
  if (!registry.contains(key)) {
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
  using Pybind11Type = Pybind11T;

  explicit Bind_TopFunction(EntityScope parent_h) {}

  virtual void Bind(Pybind11T &pb11_obj) {
    pb11_obj.def("TopFunction", static_cast<void (*)(int)>(&TopFunction), R"(/**
 * This is Function doc
 * @param i This is i doc
 */)");
    pb11_obj.def("TopFunction",
                 static_cast<void (*)(std::string &)>(&TopFunction));
  }

  static const char *Key() { return "TopFunction"; }
};

struct Entity_TopFunction
    : public Bind_TopFunction<std::decay_t<pybind11::module_ &>> {

  explicit Entity_TopFunction(EntityScope parent_h)
      : Bind_TopFunction<std::decay_t<pybind11::module_ &>>(parent_h),
        handle{static_cast<pybind11::module_ &>(parent_h)} {}

  void Update() override { Bind(handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::module_ &handle;
};

template <class Pybind11T> struct Bind_TopFunctionDef : public EntityBase {
  using Pybind11Type = Pybind11T;

  explicit Bind_TopFunctionDef(EntityScope parent_h) {}

  virtual void Bind(Pybind11T &pb11_obj) {
    pb11_obj.def("TopFunctionDef", static_cast<void (*)()>(&TopFunctionDef));
  }

  static const char *Key() { return "TopFunctionDef"; }
};

struct Entity_TopFunctionDef
    : public Bind_TopFunctionDef<std::decay_t<pybind11::module_ &>> {

  explicit Entity_TopFunctionDef(EntityScope parent_h)
      : Bind_TopFunctionDef<std::decay_t<pybind11::module_ &>>(parent_h),
        handle{static_cast<pybind11::module_ &>(parent_h)} {}

  void Update() override { Bind(handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::module_ &handle;
};

template <class Pybind11T> struct Bind_earth : public EntityBase {
  using Pybind11Type = Pybind11T;

  explicit Bind_earth(EntityScope parent_h) {}

  virtual void Bind(Pybind11T &pb11_obj) {}

  static const char *Key() { return "earth"; }
};

struct Entity_earth : public Bind_earth<std::decay_t<pybind11::module_>> {

  explicit Entity_earth(EntityScope parent_h)
      : Bind_earth<std::decay_t<pybind11::module_>>(parent_h),
        handle{
            static_cast<pybind11::module_ &>(parent_h).def_submodule("earth")} {
  }

  void Update() override { Bind(handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::module_ handle;
};

template <class Pybind11T> struct Bind_earth_creatures : public EntityBase {
  using Pybind11Type = Pybind11T;

  explicit Bind_earth_creatures(EntityScope parent_h) {}

  virtual void Bind(Pybind11T &pb11_obj) {}

  static const char *Key() { return "earth::creatures"; }
};

struct Entity_earth_creatures
    : public Bind_earth_creatures<std::decay_t<pybind11::module_>> {

  explicit Entity_earth_creatures(EntityScope parent_h)
      : Bind_earth_creatures<std::decay_t<pybind11::module_>>(parent_h),
        handle{static_cast<pybind11::module_ &>(parent_h).def_submodule(
            "creatures")} {}

  void Update() override { Bind(handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::module_ handle;
};

template <class Pybind11T>
struct Bind_earth_creatures_NSFunction : public EntityBase {
  using Pybind11Type = Pybind11T;

  explicit Bind_earth_creatures_NSFunction(EntityScope parent_h) {}

  virtual void Bind(Pybind11T &pb11_obj) {
    pb11_obj.def("NSFunction", static_cast<int (*)(const std::string &)>(
                                   &earth::creatures::NSFunction));
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

  void Update() override { Bind(handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::module_ &handle;
};

template <class Pybind11T>
struct Bind_earth_creatures_NSFunctionDef : public EntityBase {
  using Pybind11Type = Pybind11T;

  explicit Bind_earth_creatures_NSFunctionDef(EntityScope parent_h) {}

  virtual void Bind(Pybind11T &pb11_obj) {
    pb11_obj.def("NSFunctionDef",
                 static_cast<void (*)()>(&earth::creatures::NSFunctionDef));
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

  void Update() override { Bind(handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::module_ &handle;
};

template <class Pybind11T>
struct Bind_earth_creatures_Animal : public EntityBase {
  using Pybind11Type = Pybind11T;

  explicit Bind_earth_creatures_Animal(EntityScope parent_h) {}

  virtual void Bind(Pybind11T &pb11_obj) {
    pb11_obj.value("DOG", earth::creatures::Animal::DOG,
                   R"(///< This is Enum Item)");
    pb11_obj.value("CAT", earth::creatures::Animal::CAT, R"(///< Cat doc)");
  }

  static const char *Key() { return "earth::creatures::Animal"; }
};

struct Entity_earth_creatures_Animal
    : public Bind_earth_creatures_Animal<
          std::decay_t<pybind11::enum_<earth::creatures::Animal>>> {

  explicit Entity_earth_creatures_Animal(EntityScope parent_h)
      : Bind_earth_creatures_Animal<
            std::decay_t<pybind11::enum_<earth::creatures::Animal>>>(parent_h),
        handle{parent_h, "Animal", pybind11::arithmetic(),
               R"(/// This is Enum doc)"} {}

  void Update() override { Bind(handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::enum_<earth::creatures::Animal> handle;
};

template <class Pybind11T>
struct Bind_earth_creatures_ValueSet : public EntityBase {
  using Pybind11Type = Pybind11T;

  explicit Bind_earth_creatures_ValueSet(EntityScope parent_h) {}

  virtual void Bind(Pybind11T &pb11_obj) {
    pb11_obj.value("LOW", earth::creatures::ValueSet::LOW);
    pb11_obj.value("MIDDLE", earth::creatures::ValueSet::MIDDLE);
    pb11_obj.value("HIGH", earth::creatures::ValueSet::HIGH);
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

  void Update() override { Bind(handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::enum_<earth::creatures::ValueSet> handle;
};

template <class Pybind11T>
struct Bind_earth_creatures_Home : public EntityBase {
  using Pybind11Type = Pybind11T;

  explicit Bind_earth_creatures_Home(EntityScope parent_h) {}

  virtual void Bind(Pybind11T &pb11_obj) {
    pb11_obj.def(pybind11::init<>());
    Pybind11WeaverBindAllMethods(pb11_obj);
  }

  void Pybind11WeaverBindAllMethods(Pybind11T &obj) { BindMethod_Method(obj); }

  virtual void BindMethod_Method(Pybind11T &obj) {
    obj.def("Method",
            static_cast<double (earth::creatures::Home::*)(std::string &, int)>(
                &earth::creatures::Home::Method));
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

  void Update() override { Bind(handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::class_<earth::creatures::Home> handle;
};

template <class Pybind11T>
struct Bind_earth_creatures_Home_Food : public EntityBase {
  using Pybind11Type = Pybind11T;

  explicit Bind_earth_creatures_Home_Food(EntityScope parent_h) {}

  virtual void Bind(Pybind11T &pb11_obj) {
    pb11_obj.value("MEAT", earth::creatures::Home::Food::MEAT);
    pb11_obj.value("RICE", earth::creatures::Home::Food::RICE);
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

  void Update() override { Bind(handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::enum_<earth::creatures::Home::Food> handle;
};

template <class Pybind11T>
struct Bind_earth_creatures_SweetHome : public EntityBase {
  using Pybind11Type = Pybind11T;

  explicit Bind_earth_creatures_SweetHome(EntityScope parent_h) {}

  virtual void Bind(Pybind11T &pb11_obj) {
    pb11_obj.def(
        pybind11::init<int, float, const std::string &, std::string *>());
    Pybind11WeaverBindAllMethods(pb11_obj);
    pb11_obj.def_readwrite("member", &earth::creatures::SweetHome::member,
                           R"(///< This is Member doc)");
  }

  void Pybind11WeaverBindAllMethods(Pybind11T &obj) {
    BindMethod_Method(obj);
    BindMethod_VirtualMethod(obj);
    BindMethod_StaticMethod(obj);
  }

  virtual void BindMethod_Method(Pybind11T &obj) {
    obj.def("Method",
            static_cast<int (earth::creatures::SweetHome::*)(int)>(
                &earth::creatures::SweetHome::Method),
            R"(/**
   * This is Method doc
   * @return
   */)");
    obj.def("Method",
            static_cast<void (earth::creatures::SweetHome::*)(std::string &)>(
                &earth::creatures::SweetHome::Method));
  }

  virtual void BindMethod_VirtualMethod(Pybind11T &obj) {
    obj.def("VirtualMethod",
            static_cast<void (earth::creatures::SweetHome::*)(int)>(
                &earth::creatures::SweetHome::VirtualMethod));
    obj.def("VirtualMethod",
            static_cast<void (earth::creatures::SweetHome::*)(std::string &)>(
                &earth::creatures::SweetHome::VirtualMethod));
  }

  virtual void BindMethod_StaticMethod(Pybind11T &obj) {
    obj.def_static(
        "StaticMethod",
        static_cast<void (*)(int)>(&earth::creatures::SweetHome::StaticMethod));
    obj.def_static("StaticMethod",
                   static_cast<void (*)(std::string &)>(
                       &earth::creatures::SweetHome::StaticMethod));
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
        handle{parent_h, "SweetHome", R"(/**
 * This is Class doc
 */)"} {}

  void Update() override { Bind(handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::class_<earth::creatures::SweetHome> handle;
};

template <class Pybind11T> struct Bind_disabled_space : public EntityBase {
  using Pybind11Type = Pybind11T;

  explicit Bind_disabled_space(EntityScope parent_h) {}

  virtual void Bind(Pybind11T &pb11_obj) {}

  static const char *Key() { return "disabled_space"; }
};

struct Entity_disabled_space
    : public Bind_disabled_space<std::decay_t<pybind11::module_>> {

  explicit Entity_disabled_space(EntityScope parent_h)
      : Bind_disabled_space<std::decay_t<pybind11::module_>>(parent_h),
        handle{static_cast<pybind11::module_ &>(parent_h).def_submodule(
            "disabled_space")} {}

  void Update() override { Bind(handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::module_ handle;
};

template <class Pybind11T> struct Bind_disabled_space_Foo : public EntityBase {
  using Pybind11Type = Pybind11T;

  explicit Bind_disabled_space_Foo(EntityScope parent_h) {}

  virtual void Bind(Pybind11T &pb11_obj) {
    pb11_obj.value("BAR", disabled_space::Foo::BAR);
    pb11_obj.value("BAZ", disabled_space::Foo::BAZ);
  }

  static const char *Key() { return "disabled_space::Foo"; }
};

struct Entity_disabled_space_Foo
    : public Bind_disabled_space_Foo<
          std::decay_t<pybind11::enum_<disabled_space::Foo>>> {

  explicit Entity_disabled_space_Foo(EntityScope parent_h)
      : Bind_disabled_space_Foo<
            std::decay_t<pybind11::enum_<disabled_space::Foo>>>(parent_h),
        handle{parent_h, "Foo", pybind11::arithmetic()} {}

  void Update() override { Bind(handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::enum_<disabled_space::Foo> handle;
};

template <class Pybind11T> struct Bind_disabled_member : public EntityBase {
  using Pybind11Type = Pybind11T;

  explicit Bind_disabled_member(EntityScope parent_h) {}

  virtual void Bind(Pybind11T &pb11_obj) {}

  static const char *Key() { return "disabled_member"; }
};

struct Entity_disabled_member
    : public Bind_disabled_member<std::decay_t<pybind11::module_>> {

  explicit Entity_disabled_member(EntityScope parent_h)
      : Bind_disabled_member<std::decay_t<pybind11::module_>>(parent_h),
        handle{static_cast<pybind11::module_ &>(parent_h).def_submodule(
            "disabled_member")} {}

  void Update() override { Bind(handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::module_ handle;
};

template <class Pybind11T>
struct Bind_disabled_member_disabled_Foo : public EntityBase {
  using Pybind11Type = Pybind11T;

  explicit Bind_disabled_member_disabled_Foo(EntityScope parent_h) {}

  virtual void Bind(Pybind11T &pb11_obj) {
    pb11_obj.value("BAR", disabled_member::disabled_Foo::BAR);
    pb11_obj.value("BAZ", disabled_member::disabled_Foo::BAZ);
  }

  static const char *Key() { return "disabled_member::disabled_Foo"; }
};

struct Entity_disabled_member_disabled_Foo
    : public Bind_disabled_member_disabled_Foo<
          std::decay_t<pybind11::enum_<disabled_member::disabled_Foo>>> {

  explicit Entity_disabled_member_disabled_Foo(EntityScope parent_h)
      : Bind_disabled_member_disabled_Foo<
            std::decay_t<pybind11::enum_<disabled_member::disabled_Foo>>>(
            parent_h),
        handle{parent_h, "disabled_Foo", pybind11::arithmetic()} {}

  void Update() override { Bind(handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::enum_<disabled_member::disabled_Foo> handle;
};

/**
 * Create all entities, return a callable guard that can be called to update all
 *entities. If the returned guard is not called, the guard will call the update
 *function on its destruction.
 **/
[[nodiscard]] pybind11_weaver::CallUpdateGuard
DeclFn(pybind11::module &m,
       const pybind11_weaver::CustomBindingRegistry &registry) {
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
  auto v11 = pybind11_weaver::CreateEntity<Entity_disabled_space>(
      EntityScope(m), registry);
  auto v12 = pybind11_weaver::CreateEntity<Entity_disabled_space_Foo>(
      v11->AsScope(), registry);
  auto v13 = pybind11_weaver::CreateEntity<Entity_disabled_member>(
      EntityScope(m), registry);
  auto v14 = pybind11_weaver::CreateEntity<Entity_disabled_member_disabled_Foo>(
      v13->AsScope(), registry);

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
    v11->Update();
    v12->Update();
    v13->Update();
    v14->Update();
  };
  return {update_fn};
}

} // anonymous namespace
