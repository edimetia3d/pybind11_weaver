
// GENERATED AT 10/27/2023, 12:26:29

#include "sample.h"
#include "template_pb11_weaver_helper.h"

#ifndef GITHUB_COM_PYBIND11_WEAVER
#define GITHUB_COM_PYBIND11_WEAVER
#include <atomic>
#include <functional>
#include <map>
#include <mutex>
#include <thread>
#include <utility>

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace pybind11_weaver {

template <class BindT, class PB11T> void TryAddDefaultCtor(PB11T &handle) {
  if constexpr (std::is_default_constructible<BindT>::value) {
    handle.def(pybind11::init<>());
  }
}

template <class T>
void EnsureExportUsedType(pybind11::module_ &m, const char *realname) {
  using DT = std::decay_t<T>;
  if (!pybind11::detail::get_type_info(typeid(DT))) {
    std::string name = std::string("PWCapsule") + typeid(DT).name();
    pybind11::class_<DT> handle(m, name.c_str(), pybind11::module_local());
    handle.def("real_name", [=]() { return realname; });
    TryAddDefaultCtor<DT>(handle);
  }
}

template <class ClassT, class MethodRetT, class... MethodArgs> struct FnPtrT {
  using type = MethodRetT (ClassT::*)(MethodArgs...);
};
template <class FnTtype> struct FnPtrT<void, FnTtype> {
  using type = FnTtype *;
};

struct _PointerWrapperBase {
  _PointerWrapperBase(void *ptr_) : ptr(ptr_) {}
  _PointerWrapperBase(intptr_t ptr_v) : ptr(reinterpret_cast<void *>(ptr_v)) {}
  intptr_t get_ptr() { return reinterpret_cast<intptr_t>(ptr); }
  void set_ptr(intptr_t ptr_v) { ptr = reinterpret_cast<void *>(ptr_v); }
  static void FastBind(pybind11::module &m) {
    pybind11::class_<_PointerWrapperBase, std::shared_ptr<_PointerWrapperBase>>(
        m, "_PointerWrapperBase", pybind11::dynamic_attr())
        .def(pybind11::init<void *>())
        .def(pybind11::init<intptr_t>())
        .def("get_ptr", &_PointerWrapperBase::get_ptr)
        .def("set_ptr", &_PointerWrapperBase::set_ptr);
  }
  void *ptr;
};

template <class T> struct PointerWrapper : public _PointerWrapperBase {
  static_assert(std::is_pointer<T>::value, "T must be a pointer type");
  using _PointerWrapperBase::_PointerWrapperBase;

  static void FastBind(pybind11::module &m, const std::string &name) {
    pybind11::class_<PointerWrapper, std::shared_ptr<PointerWrapper>,
                     _PointerWrapperBase>(m, name.c_str(),
                                          pybind11::dynamic_attr())
        .def(pybind11::init<intptr_t>())
        .def_static("from_capsule", [](pybind11::capsule o) {
          return std::make_shared<PointerWrapper<T>>(
              reinterpret_cast<void *>(o.get_pointer()));
        });
  }
  T Cptr() { return reinterpret_cast<T>(ptr); }
};
template <class T> using WrappedPtrT = std::shared_ptr<PointerWrapper<T>>;

template <class T> WrappedPtrT<T> WrapP(T ptr) {
  if (!ptr) {
    return nullptr;
  }
  return WrappedPtrT<T>{new PointerWrapper<T>(ptr)};
}

struct Guardian {
  std::vector<std::function<void()>> dtor_callbacks;
  ~Guardian() {
    for (auto &fn : dtor_callbacks) {
      fn();
    }
  }
};

template <typename R, typename... Args> struct FnPointerWrapper {
  using CppFnT = R(Args...);

  template <class CR, typename... CArgs> struct GetCptr {
    using CFnPtrT = CR (*)(CArgs...);
    static CFnPtrT Run(std::function<CppFnT> to_call, Guardian &&guard,
                       CFnPtrT c_wrapper, const char *uuid0, int64_t uuid1) {
      // lock
      guard.dtor_callbacks.push_back(
          [uuid0, uuid1]() { ReleaseFnProxy(uuid0, uuid1); });
      SetFnProxy(uuid0, uuid1, to_call);
      return c_wrapper;
    }
  };

  using FnMapT =
      std::map<const char *, std::map<int64_t, std::function<CppFnT>>>;
  static void SetFnProxy(const char *uuid0, int64_t uuid1,
                         std::function<CppFnT> &fn) {
    FnMapMutex().lock();
    while (FnMap()[uuid0].count(uuid1) != 0) {
      // The chance is so low, spin lock should be fine
      FnMapMutex().unlock();
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      FnMapMutex().lock();
    }
    FnMap()[uuid0][uuid1] = fn;
    FnMapMutex().unlock();
  }

  static std::function<CppFnT> GetFnProxy(const char *uuid0, int64_t uuid1) {
    std::lock_guard<std::mutex> _(FnMapMutex());
    auto ret = FnMap()[uuid0][uuid1];
    return ret;
  }

  static void ReleaseFnProxy(const char *uuid0, int64_t uuid1) {
    std::lock_guard<std::mutex> _(FnMapMutex());
    FnMap()[uuid0].erase(uuid1);
  }
  static FnMapT &FnMap() {
    static FnMapT fns;
    return fns;
  }

  static std::mutex &FnMapMutex() {
    static std::mutex m;
    return m;
  }
};

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

#ifndef PB11_WEAVER_DISABLE_Entity_Foo_Q_R6int9_8
template <class Pybind11T> struct Bind_Foo_Q_R6int9_8 : public EntityBase {
  using Pybind11Type = Pybind11T;

  explicit Bind_Foo_Q_R6int9_8(EntityScope parent_h) {}

  virtual void Bind(Pybind11T &pb11_obj) {
    pb11_obj.def("Foo_Q_R6int9_8",
                 static_cast<pybind11_weaver::FnPtrT<
                     void, std::basic_string<char>(Q::R<int>, int)>::type>(
                     &Foo<Q::R<int>, 8>),
                 R"_pb11_weaver(/**
 * Two things need to be done to bind a template function:
 * 1. Use syntax of "template specialization" to force generate the binding of
 * template function.
 * 2. Use syntax of extern template to avoid duplicate symbol.
 * Note that some instance must exist in the translation unit.
 */)_pb11_weaver");
  }

  static const char *Key() { return "Foo_Q_R6int9_8"; }
};

struct Entity_Foo_Q_R6int9_8
    : public Bind_Foo_Q_R6int9_8<std::decay_t<pybind11::module_ &>> {

  explicit Entity_Foo_Q_R6int9_8(EntityScope parent_h)
      : Bind_Foo_Q_R6int9_8<std::decay_t<pybind11::module_ &>>(parent_h),
        handle{static_cast<pybind11::module_ &>(parent_h)} {}

  void Update() override { Bind(handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::module_ &handle;
};
#endif // PB11_WEAVER_DISABLE_Entity_Foo_Q_R6int9_8

#ifndef PB11_WEAVER_DISABLE_Entity_Foo_float_9
template <class Pybind11T> struct Bind_Foo_float_9 : public EntityBase {
  using Pybind11Type = Pybind11T;

  explicit Bind_Foo_float_9(EntityScope parent_h) {}

  virtual void Bind(Pybind11T &pb11_obj) {
    pb11_obj.def(
        "Foo_float_9",
        static_cast<pybind11_weaver::FnPtrT<
            void, std::basic_string<char>(float, int)>::type>(&Foo<float, 9>),
        R"_pb11_weaver(/**
 * Another one
 */)_pb11_weaver");
  }

  static const char *Key() { return "Foo_float_9"; }
};

struct Entity_Foo_float_9
    : public Bind_Foo_float_9<std::decay_t<pybind11::module_ &>> {

  explicit Entity_Foo_float_9(EntityScope parent_h)
      : Bind_Foo_float_9<std::decay_t<pybind11::module_ &>>(parent_h),
        handle{static_cast<pybind11::module_ &>(parent_h)} {}

  void Update() override { Bind(handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::module_ &handle;
};
#endif // PB11_WEAVER_DISABLE_Entity_Foo_float_9

#ifndef PB11_WEAVER_DISABLE_Entity_GetNotBoundType
template <class Pybind11T> struct Bind_GetNotBoundType : public EntityBase {
  using Pybind11Type = Pybind11T;

  explicit Bind_GetNotBoundType(EntityScope parent_h) {}

  virtual void Bind(Pybind11T &pb11_obj) {
    pb11_obj.def(
        "GetNotBoundType",
        static_cast<pybind11_weaver::FnPtrT<void, UnexportedType *()>::type>(
            &GetNotBoundType));
  }

  static const char *Key() { return "GetNotBoundType"; }
};

struct Entity_GetNotBoundType
    : public Bind_GetNotBoundType<std::decay_t<pybind11::module_ &>> {

  explicit Entity_GetNotBoundType(EntityScope parent_h)
      : Bind_GetNotBoundType<std::decay_t<pybind11::module_ &>>(parent_h),
        handle{static_cast<pybind11::module_ &>(parent_h)} {}

  void Update() override { Bind(handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::module_ &handle;
};
#endif // PB11_WEAVER_DISABLE_Entity_GetNotBoundType

#ifndef PB11_WEAVER_DISABLE_Entity_Q
template <class Pybind11T> struct Bind_Q : public EntityBase {
  using Pybind11Type = Pybind11T;

  explicit Bind_Q(EntityScope parent_h) {}

  virtual void Bind(Pybind11T &pb11_obj) {}

  static const char *Key() { return "Q"; }
};

struct Entity_Q : public Bind_Q<std::decay_t<pybind11::module_>> {

  explicit Entity_Q(EntityScope parent_h)
      : Bind_Q<std::decay_t<pybind11::module_>>(parent_h),
        handle{static_cast<pybind11::module_ &>(parent_h).def_submodule("Q")} {}

  void Update() override { Bind(handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::module_ handle;
};
#endif // PB11_WEAVER_DISABLE_Entity_Q

#ifndef PB11_WEAVER_DISABLE_Entity_Q_R6int9
template <class Pybind11T> struct Bind_Q_R6int9 : public EntityBase {
  using Pybind11Type = Pybind11T;

  using T = int;

  void Pybind11WeaverBindAllMethods(Pybind11T &obj) {}

  explicit Bind_Q_R6int9(EntityScope parent_h) {}

  virtual void Bind(Pybind11T &pb11_obj) {
    pybind11_weaver::TryAddDefaultCtor<Q::R<int>>(pb11_obj);
    Pybind11WeaverBindAllMethods(pb11_obj);
  }

  static const char *Key() { return "Q_R6int9"; }
};

struct Entity_Q_R6int9
    : public Bind_Q_R6int9<std::decay_t<pybind11::class_<Q::R<int>>>> {

  explicit Entity_Q_R6int9(EntityScope parent_h)
      : Bind_Q_R6int9<std::decay_t<pybind11::class_<Q::R<int>>>>(parent_h),
        handle{parent_h, "R6int9", pybind11::dynamic_attr()} {}

  void Update() override { Bind(handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::class_<Q::R<int>> handle;
};
#endif // PB11_WEAVER_DISABLE_Entity_Q_R6int9

#ifndef PB11_WEAVER_DISABLE_Entity_template_ns_TemplateClass6double9
template <class Pybind11T>
struct Bind_template_ns_TemplateClass6double9 : public EntityBase {
  using Pybind11Type = Pybind11T;

  using T = double;

  void Pybind11WeaverBindAllMethods(Pybind11T &obj) { BindMethod_Method(obj); }

  virtual void BindMethod_Method(Pybind11T &obj) {
    obj.def("Method",
            static_cast<pybind11_weaver::FnPtrT<template_ns::TemplateClass<T>,
                                                T, T *>::type>(
                &template_ns::TemplateClass<T>::Method));
  }

  explicit Bind_template_ns_TemplateClass6double9(EntityScope parent_h) {}

  virtual void Bind(Pybind11T &pb11_obj) {
    pybind11_weaver::TryAddDefaultCtor<template_ns::TemplateClass<double>>(
        pb11_obj);
    Pybind11WeaverBindAllMethods(pb11_obj);
  }

  static const char *Key() { return "template_ns_TemplateClass6double9"; }
};

struct Entity_template_ns_TemplateClass6double9
    : public Bind_template_ns_TemplateClass6double9<
          std::decay_t<pybind11::class_<template_ns::TemplateClass<double>>>> {

  explicit Entity_template_ns_TemplateClass6double9(EntityScope parent_h)
      : Bind_template_ns_TemplateClass6double9<
            std::decay_t<pybind11::class_<template_ns::TemplateClass<double>>>>(
            parent_h),
        handle{parent_h, "TemplateClass6double9", pybind11::dynamic_attr()} {}

  void Update() override { Bind(handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::class_<template_ns::TemplateClass<double>> handle;
};
#endif // PB11_WEAVER_DISABLE_Entity_template_ns_TemplateClass6double9

#ifndef PB11_WEAVER_DISABLE_Entity_TopFunction
template <class Pybind11T> struct Bind_TopFunction : public EntityBase {
  using Pybind11Type = Pybind11T;

  explicit Bind_TopFunction(EntityScope parent_h) {}

  virtual void Bind(Pybind11T &pb11_obj) {
    pb11_obj.def("TopFunction",
                 static_cast<pybind11_weaver::FnPtrT<void, void(int)>::type>(
                     &TopFunction),
                 R"_pb11_weaver(/**
 * This is Function doc
 * @param i This is i doc
 */)_pb11_weaver");
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
#endif // PB11_WEAVER_DISABLE_Entity_TopFunction

#ifndef PB11_WEAVER_DISABLE_Entity_TopFunction1
template <class Pybind11T> struct Bind_TopFunction1 : public EntityBase {
  using Pybind11Type = Pybind11T;

  explicit Bind_TopFunction1(EntityScope parent_h) {}

  virtual void Bind(Pybind11T &pb11_obj) {
    pb11_obj.def(
        "TopFunction",
        static_cast<pybind11_weaver::FnPtrT<
            void, void(std::basic_string<char> &)>::type>(&TopFunction));
  }

  static const char *Key() { return "TopFunction1"; }
};

struct Entity_TopFunction1
    : public Bind_TopFunction1<std::decay_t<pybind11::module_ &>> {

  explicit Entity_TopFunction1(EntityScope parent_h)
      : Bind_TopFunction1<std::decay_t<pybind11::module_ &>>(parent_h),
        handle{static_cast<pybind11::module_ &>(parent_h)} {}

  void Update() override { Bind(handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::module_ &handle;
};
#endif // PB11_WEAVER_DISABLE_Entity_TopFunction1

#ifndef PB11_WEAVER_DISABLE_Entity_TopFunctionDef
template <class Pybind11T> struct Bind_TopFunctionDef : public EntityBase {
  using Pybind11Type = Pybind11T;

  explicit Bind_TopFunctionDef(EntityScope parent_h) {}

  virtual void Bind(Pybind11T &pb11_obj) {
    pb11_obj.def("TopFunctionDef",
                 static_cast<pybind11_weaver::FnPtrT<void, void()>::type>(
                     &TopFunctionDef));
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
#endif // PB11_WEAVER_DISABLE_Entity_TopFunctionDef

#ifndef PB11_WEAVER_DISABLE_Entity_disabled_member
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
#endif // PB11_WEAVER_DISABLE_Entity_disabled_member

#ifndef PB11_WEAVER_DISABLE_Entity_disabled_member_disabled_Foo
template <class Pybind11T>
struct Bind_disabled_member_disabled_Foo : public EntityBase {
  using Pybind11Type = Pybind11T;

  explicit Bind_disabled_member_disabled_Foo(EntityScope parent_h) {}

  virtual void Bind(Pybind11T &pb11_obj) {
    pb11_obj.value("BAR", disabled_member::disabled_Foo::BAR);
    pb11_obj.value("BAZ", disabled_member::disabled_Foo::BAZ);
  }

  static const char *Key() { return "disabled_member_disabled_Foo"; }
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
#endif // PB11_WEAVER_DISABLE_Entity_disabled_member_disabled_Foo

#ifndef PB11_WEAVER_DISABLE_Entity_disabled_space
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
#endif // PB11_WEAVER_DISABLE_Entity_disabled_space

#ifndef PB11_WEAVER_DISABLE_Entity_disabled_space_Foo
template <class Pybind11T> struct Bind_disabled_space_Foo : public EntityBase {
  using Pybind11Type = Pybind11T;

  explicit Bind_disabled_space_Foo(EntityScope parent_h) {}

  virtual void Bind(Pybind11T &pb11_obj) {
    pb11_obj.value("BAR", disabled_space::Foo::BAR);
    pb11_obj.value("BAZ", disabled_space::Foo::BAZ);
  }

  static const char *Key() { return "disabled_space_Foo"; }
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
#endif // PB11_WEAVER_DISABLE_Entity_disabled_space_Foo

#ifndef PB11_WEAVER_DISABLE_Entity_earth
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
#endif // PB11_WEAVER_DISABLE_Entity_earth

#ifndef PB11_WEAVER_DISABLE_Entity_earth_creatures
template <class Pybind11T> struct Bind_earth_creatures : public EntityBase {
  using Pybind11Type = Pybind11T;

  explicit Bind_earth_creatures(EntityScope parent_h) {}

  virtual void Bind(Pybind11T &pb11_obj) {}

  static const char *Key() { return "earth_creatures"; }
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
#endif // PB11_WEAVER_DISABLE_Entity_earth_creatures

#ifndef PB11_WEAVER_DISABLE_Entity_earth_creatures_Animal
template <class Pybind11T>
struct Bind_earth_creatures_Animal : public EntityBase {
  using Pybind11Type = Pybind11T;

  explicit Bind_earth_creatures_Animal(EntityScope parent_h) {}

  virtual void Bind(Pybind11T &pb11_obj) {
    pb11_obj.value("DOG", earth::creatures::Animal::DOG,
                   R"_pb11_weaver(///< This is Enum Item)_pb11_weaver");
    pb11_obj.value("CAT", earth::creatures::Animal::CAT,
                   R"_pb11_weaver(///< Cat doc)_pb11_weaver");
  }

  static const char *Key() { return "earth_creatures_Animal"; }
};

struct Entity_earth_creatures_Animal
    : public Bind_earth_creatures_Animal<
          std::decay_t<pybind11::enum_<earth::creatures::Animal>>> {

  explicit Entity_earth_creatures_Animal(EntityScope parent_h)
      : Bind_earth_creatures_Animal<
            std::decay_t<pybind11::enum_<earth::creatures::Animal>>>(parent_h),
        handle{parent_h, "Animal", pybind11::arithmetic(),
               R"_pb11_weaver(/// This is Enum doc)_pb11_weaver"} {}

  void Update() override { Bind(handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::enum_<earth::creatures::Animal> handle;
};
#endif // PB11_WEAVER_DISABLE_Entity_earth_creatures_Animal

#ifndef PB11_WEAVER_DISABLE_Entity_earth_creatures_Home
template <class Pybind11T>
struct Bind_earth_creatures_Home : public EntityBase {
  using Pybind11Type = Pybind11T;

  void Pybind11WeaverBindAllMethods(Pybind11T &obj) { BindMethod_Method(obj); }

  virtual void BindMethod_Method(Pybind11T &obj) {
    obj.def("Method",
            static_cast<
                pybind11_weaver::FnPtrT<earth::creatures::Home, double,
                                        std::basic_string<char> &, int>::type>(
                &earth::creatures::Home::Method));
  }

  explicit Bind_earth_creatures_Home(EntityScope parent_h) {}

  virtual void Bind(Pybind11T &pb11_obj) {
    pybind11_weaver::TryAddDefaultCtor<earth::creatures::Home>(pb11_obj);
    Pybind11WeaverBindAllMethods(pb11_obj);
  }

  static const char *Key() { return "earth_creatures_Home"; }
};

struct Entity_earth_creatures_Home
    : public Bind_earth_creatures_Home<
          std::decay_t<pybind11::class_<earth::creatures::Home>>> {

  explicit Entity_earth_creatures_Home(EntityScope parent_h)
      : Bind_earth_creatures_Home<
            std::decay_t<pybind11::class_<earth::creatures::Home>>>(parent_h),
        handle{parent_h, "Home", pybind11::dynamic_attr()} {}

  void Update() override { Bind(handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::class_<earth::creatures::Home> handle;
};
#endif // PB11_WEAVER_DISABLE_Entity_earth_creatures_Home

#ifndef PB11_WEAVER_DISABLE_Entity_earth_creatures_Home_Food
template <class Pybind11T>
struct Bind_earth_creatures_Home_Food : public EntityBase {
  using Pybind11Type = Pybind11T;

  explicit Bind_earth_creatures_Home_Food(EntityScope parent_h) {}

  virtual void Bind(Pybind11T &pb11_obj) {
    pb11_obj.value("MEAT", earth::creatures::Home::Food::MEAT);
    pb11_obj.value("RICE", earth::creatures::Home::Food::RICE);
  }

  static const char *Key() { return "earth_creatures_Home_Food"; }
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
#endif // PB11_WEAVER_DISABLE_Entity_earth_creatures_Home_Food

#ifndef PB11_WEAVER_DISABLE_Entity_NSFunction
template <class Pybind11T> struct Bind_NSFunction : public EntityBase {
  using Pybind11Type = Pybind11T;

  explicit Bind_NSFunction(EntityScope parent_h) {}

  virtual void Bind(Pybind11T &pb11_obj) {
    pb11_obj.def("NSFunction",
                 static_cast<pybind11_weaver::FnPtrT<
                     void, int(const std::basic_string<char> &)>::type>(
                     &earth::creatures::NSFunction));
  }

  static const char *Key() { return "NSFunction"; }
};

struct Entity_NSFunction
    : public Bind_NSFunction<std::decay_t<pybind11::module_ &>> {

  explicit Entity_NSFunction(EntityScope parent_h)
      : Bind_NSFunction<std::decay_t<pybind11::module_ &>>(parent_h),
        handle{static_cast<pybind11::module_ &>(parent_h)} {}

  void Update() override { Bind(handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::module_ &handle;
};
#endif // PB11_WEAVER_DISABLE_Entity_NSFunction

#ifndef PB11_WEAVER_DISABLE_Entity_NSFunctionDef
template <class Pybind11T> struct Bind_NSFunctionDef : public EntityBase {
  using Pybind11Type = Pybind11T;

  explicit Bind_NSFunctionDef(EntityScope parent_h) {}

  virtual void Bind(Pybind11T &pb11_obj) {
    pb11_obj.def("NSFunctionDef",
                 static_cast<pybind11_weaver::FnPtrT<void, void()>::type>(
                     &earth::creatures::NSFunctionDef));
  }

  static const char *Key() { return "NSFunctionDef"; }
};

struct Entity_NSFunctionDef
    : public Bind_NSFunctionDef<std::decay_t<pybind11::module_ &>> {

  explicit Entity_NSFunctionDef(EntityScope parent_h)
      : Bind_NSFunctionDef<std::decay_t<pybind11::module_ &>>(parent_h),
        handle{static_cast<pybind11::module_ &>(parent_h)} {}

  void Update() override { Bind(handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::module_ &handle;
};
#endif // PB11_WEAVER_DISABLE_Entity_NSFunctionDef

#ifndef PB11_WEAVER_DISABLE_Entity_earth_creatures_SweetHome
template <class Pybind11T>
struct Bind_earth_creatures_SweetHome : public EntityBase {
  using Pybind11Type = Pybind11T;

  void Pybind11WeaverBindAllMethods(Pybind11T &obj) {
    BindMethod_Method(obj);
    BindMethod_Method1(obj);
    BindMethod_VirtualMethod(obj);
    BindMethod_VirtualMethod1(obj);
    BindMethod_StaticMethod(obj);
    BindMethod_StaticMethod1(obj);
    BindMethod_use_c_callback(obj);
    BindMethod_use_cpp_callback(obj);
  }

  virtual void BindMethod_Method(Pybind11T &obj) {
    obj.def("Method",
            static_cast<pybind11_weaver::FnPtrT<earth::creatures::SweetHome,
                                                int, int>::type>(
                &earth::creatures::SweetHome::Method),
            R"_pb11_weaver(/**
   * This is Method doc
   * @return
   */)_pb11_weaver");
  }

  virtual void BindMethod_Method1(Pybind11T &obj) {
    obj.def(
        "Method",
        static_cast<pybind11_weaver::FnPtrT<earth::creatures::SweetHome, void,
                                            std::basic_string<char> &>::type>(
            &earth::creatures::SweetHome::Method));
  }

  virtual void BindMethod_VirtualMethod(Pybind11T &obj) {
    obj.def("VirtualMethod",
            static_cast<pybind11_weaver::FnPtrT<earth::creatures::SweetHome,
                                                void, int>::type>(
                &earth::creatures::SweetHome::VirtualMethod));
  }

  virtual void BindMethod_VirtualMethod1(Pybind11T &obj) {
    obj.def(
        "VirtualMethod",
        static_cast<pybind11_weaver::FnPtrT<earth::creatures::SweetHome, void,
                                            std::basic_string<char> &>::type>(
            &earth::creatures::SweetHome::VirtualMethod));
  }

  virtual void BindMethod_StaticMethod(Pybind11T &obj) {
    obj.def_static("StaticMethod",
                   static_cast<pybind11_weaver::FnPtrT<void, void(int)>::type>(
                       &earth::creatures::SweetHome::StaticMethod));
  }

  virtual void BindMethod_StaticMethod1(Pybind11T &obj) {
    obj.def_static("StaticMethod",
                   static_cast<pybind11_weaver::FnPtrT<
                       void, void(std::basic_string<char> &)>::type>(
                       &earth::creatures::SweetHome::StaticMethod));
  }

  virtual void BindMethod_use_c_callback(Pybind11T &obj) {
    obj.def(
        "use_c_callback",
        [](earth::creatures::SweetHome &self,
           std::function<int(int, pybind11_weaver::WrappedPtrT<void *>)>
               callback) {
          return self.earth::creatures::SweetHome::use_c_callback(
              pybind11_weaver::FnPointerWrapper<
                  int, int, pybind11_weaver::WrappedPtrT<void *>>::
                  GetCptr<int, int, void *>::Run(
                      (callback), pybind11_weaver::Guardian(),
                      [](int arg0_0, void *arg0_1) {
                        auto to_call = pybind11_weaver::FnPointerWrapper<
                            int, int, pybind11_weaver::WrappedPtrT<void *>>::
                            GetFnProxy(__DATE__ __TIME__ __FILE__, __COUNTER__);
                        return to_call(arg0_0,
                                       pybind11_weaver::WrapP<void *>(arg0_1));
                      },
                      /* clang-format off */
__DATE__ __TIME__ __FILE__, 
__COUNTER__ - 1
                      /* clang-format on */
                      ));
        });
  }

  virtual void BindMethod_use_cpp_callback(Pybind11T &obj) {
    obj.def(
        "use_cpp_callback",
        [](earth::creatures::SweetHome &self,
           std::function<int(int, pybind11_weaver::WrappedPtrT<void *>)>
               callback) {
          return self.earth::creatures::SweetHome::use_cpp_callback(
              pybind11_weaver::FnPointerWrapper<
                  int, int, pybind11_weaver::WrappedPtrT<void *>>::
                  GetCptr<int, int, void *>::Run(
                      (callback), pybind11_weaver::Guardian(),
                      [](int arg0_0, void *arg0_1) {
                        auto to_call = pybind11_weaver::FnPointerWrapper<
                            int, int, pybind11_weaver::WrappedPtrT<void *>>::
                            GetFnProxy(__DATE__ __TIME__ __FILE__, __COUNTER__);
                        return to_call(arg0_0,
                                       pybind11_weaver::WrapP<void *>(arg0_1));
                      },
                      /* clang-format off */
__DATE__ __TIME__ __FILE__, 
__COUNTER__ - 1
                      /* clang-format on */
                      ));
        });
  }

  explicit Bind_earth_creatures_SweetHome(EntityScope parent_h) {}

  virtual void Bind(Pybind11T &pb11_obj) {
    pb11_obj.def(pybind11::init<int, float, const std::basic_string<char> &,
                                std::basic_string<char> *>());
    Pybind11WeaverBindAllMethods(pb11_obj);
    pb11_obj.def_readwrite(
        "member", &earth::creatures::SweetHome::member,
        R"_pb11_weaver(///< This is Member doc)_pb11_weaver");
  }

  static const char *Key() { return "earth_creatures_SweetHome"; }
};

struct Entity_earth_creatures_SweetHome
    : public Bind_earth_creatures_SweetHome<
          std::decay_t<pybind11::class_<earth::creatures::SweetHome>>> {

  explicit Entity_earth_creatures_SweetHome(EntityScope parent_h)
      : Bind_earth_creatures_SweetHome<
            std::decay_t<pybind11::class_<earth::creatures::SweetHome>>>(
            parent_h),
        handle{parent_h, "SweetHome", pybind11::dynamic_attr(),
               R"_pb11_weaver(/**
 * This is Class doc
 */)_pb11_weaver"} {}

  void Update() override { Bind(handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::class_<earth::creatures::SweetHome> handle;
};
#endif // PB11_WEAVER_DISABLE_Entity_earth_creatures_SweetHome

#ifndef PB11_WEAVER_DISABLE_Entity_earth_creatures_ValueSet
template <class Pybind11T>
struct Bind_earth_creatures_ValueSet : public EntityBase {
  using Pybind11Type = Pybind11T;

  explicit Bind_earth_creatures_ValueSet(EntityScope parent_h) {}

  virtual void Bind(Pybind11T &pb11_obj) {
    pb11_obj.value("LOW", earth::creatures::ValueSet::LOW);
    pb11_obj.value("MIDDLE", earth::creatures::ValueSet::MIDDLE);
    pb11_obj.value("HIGH", earth::creatures::ValueSet::HIGH);
  }

  static const char *Key() { return "earth_creatures_ValueSet"; }
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
#endif // PB11_WEAVER_DISABLE_Entity_earth_creatures_ValueSet

#ifndef PB11_WEAVER_DISABLE_Entity_template_ns
template <class Pybind11T> struct Bind_template_ns : public EntityBase {
  using Pybind11Type = Pybind11T;

  explicit Bind_template_ns(EntityScope parent_h) {}

  virtual void Bind(Pybind11T &pb11_obj) {}

  static const char *Key() { return "template_ns"; }
};

struct Entity_template_ns
    : public Bind_template_ns<std::decay_t<pybind11::module_>> {

  explicit Entity_template_ns(EntityScope parent_h)
      : Bind_template_ns<std::decay_t<pybind11::module_>>(parent_h),
        handle{static_cast<pybind11::module_ &>(parent_h).def_submodule(
            "template_ns")} {}

  void Update() override { Bind(handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::module_ handle;
};
#endif // PB11_WEAVER_DISABLE_Entity_template_ns

#ifndef PB11_WEAVER_DISABLE_Entity_template_ns_DerivedClass
template <class Pybind11T>
struct Bind_template_ns_DerivedClass : public EntityBase {
  using Pybind11Type = Pybind11T;

  void Pybind11WeaverBindAllMethods(Pybind11T &obj) { BindMethod_Derived(obj); }

  virtual void BindMethod_Derived(Pybind11T &obj) {
    obj.def(
        "Derived",
        static_cast<
            pybind11_weaver::FnPtrT<template_ns::DerivedClass, double>::type>(
            &template_ns::DerivedClass::Derived));
  }

  explicit Bind_template_ns_DerivedClass(EntityScope parent_h) {}

  virtual void Bind(Pybind11T &pb11_obj) {
    pybind11_weaver::TryAddDefaultCtor<template_ns::DerivedClass>(pb11_obj);
    Pybind11WeaverBindAllMethods(pb11_obj);
  }

  static const char *Key() { return "template_ns_DerivedClass"; }
};

struct Entity_template_ns_DerivedClass
    : public Bind_template_ns_DerivedClass<std::decay_t<pybind11::class_<
          template_ns::DerivedClass, template_ns::TemplateClass<double>>>> {

  explicit Entity_template_ns_DerivedClass(EntityScope parent_h)
      : Bind_template_ns_DerivedClass<std::decay_t<pybind11::class_<
            template_ns::DerivedClass, template_ns::TemplateClass<double>>>>(
            parent_h),
        handle{parent_h, "DerivedClass", pybind11::dynamic_attr()} {}

  void Update() override { Bind(handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::class_<template_ns::DerivedClass,
                   template_ns::TemplateClass<double>>
      handle;
};
#endif // PB11_WEAVER_DISABLE_Entity_template_ns_DerivedClass

#ifndef PB11_WEAVER_DISABLE_Entity_template_ns_TemplateClass6float9
template <class Pybind11T>
struct Bind_template_ns_TemplateClass6float9 : public EntityBase {
  using Pybind11Type = Pybind11T;

  using T = float;

  void Pybind11WeaverBindAllMethods(Pybind11T &obj) { BindMethod_Get(obj); }

  virtual void BindMethod_Get(Pybind11T &obj) {
    obj.def("Get", static_cast<pybind11_weaver::FnPtrT<
                       template_ns::TemplateClass<float>, float>::type>(
                       &template_ns::TemplateClass<float>::Get));
  }

  explicit Bind_template_ns_TemplateClass6float9(EntityScope parent_h) {}

  virtual void Bind(Pybind11T &pb11_obj) {
    pybind11_weaver::TryAddDefaultCtor<template_ns::TemplateClass<float>>(
        pb11_obj);
    Pybind11WeaverBindAllMethods(pb11_obj);
  }

  static const char *Key() { return "template_ns_TemplateClass6float9"; }
};

struct Entity_template_ns_TemplateClass6float9
    : public Bind_template_ns_TemplateClass6float9<
          std::decay_t<pybind11::class_<template_ns::TemplateClass<float>>>> {

  explicit Entity_template_ns_TemplateClass6float9(EntityScope parent_h)
      : Bind_template_ns_TemplateClass6float9<
            std::decay_t<pybind11::class_<template_ns::TemplateClass<float>>>>(
            parent_h),
        handle{parent_h, "TemplateClass6float9", pybind11::dynamic_attr(),
               R"_pb11_weaver(// Specialization is supported directly
// extra binding could be enabled by using extern explicit instantiation)_pb11_weaver"} {
  }

  void Update() override { Bind(handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::class_<template_ns::TemplateClass<float>> handle;
};
#endif // PB11_WEAVER_DISABLE_Entity_template_ns_TemplateClass6float9

#ifndef PB11_WEAVER_DISABLE_Entity_template_ns_TemplateClass6int9
template <class Pybind11T>
struct Bind_template_ns_TemplateClass6int9 : public EntityBase {
  using Pybind11Type = Pybind11T;

  using T = int;

  void Pybind11WeaverBindAllMethods(Pybind11T &obj) { BindMethod_Method(obj); }

  virtual void BindMethod_Method(Pybind11T &obj) {
    obj.def("Method",
            static_cast<pybind11_weaver::FnPtrT<template_ns::TemplateClass<T>,
                                                T, T *>::type>(
                &template_ns::TemplateClass<T>::Method));
  }

  explicit Bind_template_ns_TemplateClass6int9(EntityScope parent_h) {}

  virtual void Bind(Pybind11T &pb11_obj) {
    pybind11_weaver::TryAddDefaultCtor<template_ns::TemplateClass<int>>(
        pb11_obj);
    Pybind11WeaverBindAllMethods(pb11_obj);
  }

  static const char *Key() { return "template_ns_TemplateClass6int9"; }
};

struct Entity_template_ns_TemplateClass6int9
    : public Bind_template_ns_TemplateClass6int9<
          std::decay_t<pybind11::class_<template_ns::TemplateClass<int>>>> {

  explicit Entity_template_ns_TemplateClass6int9(EntityScope parent_h)
      : Bind_template_ns_TemplateClass6int9<
            std::decay_t<pybind11::class_<template_ns::TemplateClass<int>>>>(
            parent_h),
        handle{
            parent_h, "TemplateClass6int9", pybind11::dynamic_attr(),
            R"_pb11_weaver(// For template class, an extern explicit instantiation is enough.)_pb11_weaver"} {
  }

  void Update() override { Bind(handle); }

  EntityScope AsScope() override { return EntityScope(handle); }

  pybind11::class_<template_ns::TemplateClass<int>> handle;
};
#endif // PB11_WEAVER_DISABLE_Entity_template_ns_TemplateClass6int9

/**
 * Create all entities, return a callable guard that can be called to update all
 *entities. If the returned guard is not called, the guard will call the update
 *function on its destruction.
 **/
[[nodiscard]] pybind11_weaver::CallUpdateGuard
DeclFn(pybind11::module &m,
       const pybind11_weaver::CustomBindingRegistry &registry) {
  pybind11_weaver::_PointerWrapperBase::FastBind(m);
  pybind11_weaver::PointerWrapper<void *>::FastBind(m, "voidp");

  auto v0 = pybind11_weaver::CreateEntity<Entity_Foo_Q_R6int9_8>(EntityScope(m),
                                                                 registry);
  auto v1 = pybind11_weaver::CreateEntity<Entity_Foo_float_9>(EntityScope(m),
                                                              registry);
  auto v2 = pybind11_weaver::CreateEntity<Entity_GetNotBoundType>(
      EntityScope(m), registry);
  auto v3 = pybind11_weaver::CreateEntity<Entity_Q>(EntityScope(m), registry);
  auto v4 =
      pybind11_weaver::CreateEntity<Entity_Q_R6int9>(EntityScope(m), registry);
  auto v5 =
      pybind11_weaver::CreateEntity<Entity_template_ns_TemplateClass6double9>(
          EntityScope(m), registry);
  auto v6 = pybind11_weaver::CreateEntity<Entity_TopFunction>(EntityScope(m),
                                                              registry);
  auto v7 = pybind11_weaver::CreateEntity<Entity_TopFunction1>(EntityScope(m),
                                                               registry);
  auto v8 = pybind11_weaver::CreateEntity<Entity_TopFunctionDef>(EntityScope(m),
                                                                 registry);
  auto v9 = pybind11_weaver::CreateEntity<Entity_disabled_member>(
      EntityScope(m), registry);
  auto v10 = pybind11_weaver::CreateEntity<Entity_disabled_member_disabled_Foo>(
      v9->AsScope(), registry);
  auto v11 = pybind11_weaver::CreateEntity<Entity_disabled_space>(
      EntityScope(m), registry);
  auto v12 = pybind11_weaver::CreateEntity<Entity_disabled_space_Foo>(
      v11->AsScope(), registry);
  auto v13 =
      pybind11_weaver::CreateEntity<Entity_earth>(EntityScope(m), registry);
  auto v14 = pybind11_weaver::CreateEntity<Entity_earth_creatures>(
      v13->AsScope(), registry);
  auto v15 = pybind11_weaver::CreateEntity<Entity_earth_creatures_Animal>(
      v14->AsScope(), registry);
  auto v16 = pybind11_weaver::CreateEntity<Entity_earth_creatures_Home>(
      v14->AsScope(), registry);
  auto v17 = pybind11_weaver::CreateEntity<Entity_earth_creatures_Home_Food>(
      v16->AsScope(), registry);
  auto v18 = pybind11_weaver::CreateEntity<Entity_NSFunction>(v14->AsScope(),
                                                              registry);
  auto v19 = pybind11_weaver::CreateEntity<Entity_NSFunctionDef>(v14->AsScope(),
                                                                 registry);
  auto v20 = pybind11_weaver::CreateEntity<Entity_earth_creatures_SweetHome>(
      v14->AsScope(), registry);
  auto v21 = pybind11_weaver::CreateEntity<Entity_earth_creatures_ValueSet>(
      v14->AsScope(), registry);
  auto v22 = pybind11_weaver::CreateEntity<Entity_template_ns>(EntityScope(m),
                                                               registry);
  auto v23 = pybind11_weaver::CreateEntity<Entity_template_ns_DerivedClass>(
      v22->AsScope(), registry);
  auto v24 =
      pybind11_weaver::CreateEntity<Entity_template_ns_TemplateClass6float9>(
          v22->AsScope(), registry);
  auto v25 =
      pybind11_weaver::CreateEntity<Entity_template_ns_TemplateClass6int9>(
          v22->AsScope(), registry);

  pybind11_weaver::EnsureExportUsedType<UnexportedType>(m, "UnexportedType");
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
    v15->Update();
    v16->Update();
    v17->Update();
    v18->Update();
    v19->Update();
    v20->Update();
    v21->Update();
    v22->Update();
    v23->Update();
    v24->Update();
    v25->Update();
  };
  return {update_fn};
}

} // anonymous namespace
