
// GENERATED AT 10/28/2023, 17:31:05

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

template <class ClassT, class MethodRetT, class... MethodArgs> struct FnPtrT {
  using type = MethodRetT (ClassT::*)(MethodArgs...);
  using const_type = MethodRetT (ClassT::*)(MethodArgs...) const;
  using const_rref_type = MethodRetT (ClassT::*)(MethodArgs...) const &&;
  using const_lref_type = MethodRetT (ClassT::*)(MethodArgs...) const &;
  using rref_type = MethodRetT (ClassT::*)(MethodArgs...) &&;
  using lref_type = MethodRetT (ClassT::*)(MethodArgs...) &;
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
  return WrappedPtrT<T>{new PointerWrapper<T>((void *)ptr)};
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

template <class Pybind11T = pybind11::module_ &>
struct Bind_Foo_Q_R6int9_8 : public EntityBase {
  using Pybind11Type = Pybind11T;

  virtual const char *AddFunction() {
    const char *_pb11_weaver_comment_str = R"_pb11_weaver(/**
 * Two things need to be done to bind a template function:
 * 1. Use syntax of "template specialization" to force generate the binding of
 * template function.
 * 2. Use syntax of extern template to avoid duplicate symbol.
 * Note that some instance must exist in the translation unit.
 */)_pb11_weaver";

#ifndef PB11_WEAVER_DISABLE_Foo_Q_R6int9_8_AddFunction
    handle.def("Foo_Q_R6int9_8",
               static_cast<pybind11_weaver::FnPtrT<
                   void, std::basic_string<char>(Q::R<int>, int)>::type>(
                   &Foo<Q::R<int>, 8>),
               _pb11_weaver_comment_str);
#endif
    return _pb11_weaver_comment_str;
  }

  explicit Bind_Foo_Q_R6int9_8(EntityScope parent_h)
      : handle{static_cast<pybind11::module_ &>(parent_h)} {}

  template <class... HandleArgsT>
  explicit Bind_Foo_Q_R6int9_8(EntityScope parent_h, HandleArgsT &&...args)
      : handle{std::forward(args)...} {}

  void Update() override { AddFunction(); }

  EntityScope AsScope() override { return EntityScope(handle); }

  static const char *Key() { return "Foo_Q_R6int9_8"; }

  Pybind11Type handle;
};
#ifndef PB11_WEAVER_DISABLE_Entity_Foo_Q_R6int9_8

using Entity_Foo_Q_R6int9_8 = Bind_Foo_Q_R6int9_8<>;

#else

struct Entity_Foo_Q_R6int9_8 : public pybind11_weaver::DisabledEntity {
  explicit Entity_Foo_Q_R6int9_8(EntityScope parent_h) {}
  static const char *Key() { return "Foo_Q_R6int9_8"; }
};

#endif // PB11_WEAVER_DISABLE_Entity_Foo_Q_R6int9_8

template <class Pybind11T = pybind11::module_ &>
struct Bind_Foo_float_9 : public EntityBase {
  using Pybind11Type = Pybind11T;

  virtual const char *AddFunction() {
    const char *_pb11_weaver_comment_str = R"_pb11_weaver(/**
 * Another one
 */)_pb11_weaver";

#ifndef PB11_WEAVER_DISABLE_Foo_float_9_AddFunction
    handle.def(
        "Foo_float_9",
        static_cast<pybind11_weaver::FnPtrT<
            void, std::basic_string<char>(float, int)>::type>(&Foo<float, 9>),
        _pb11_weaver_comment_str);
#endif
    return _pb11_weaver_comment_str;
  }

  explicit Bind_Foo_float_9(EntityScope parent_h)
      : handle{static_cast<pybind11::module_ &>(parent_h)} {}

  template <class... HandleArgsT>
  explicit Bind_Foo_float_9(EntityScope parent_h, HandleArgsT &&...args)
      : handle{std::forward(args)...} {}

  void Update() override { AddFunction(); }

  EntityScope AsScope() override { return EntityScope(handle); }

  static const char *Key() { return "Foo_float_9"; }

  Pybind11Type handle;
};
#ifndef PB11_WEAVER_DISABLE_Entity_Foo_float_9

using Entity_Foo_float_9 = Bind_Foo_float_9<>;

#else

struct Entity_Foo_float_9 : public pybind11_weaver::DisabledEntity {
  explicit Entity_Foo_float_9(EntityScope parent_h) {}
  static const char *Key() { return "Foo_float_9"; }
};

#endif // PB11_WEAVER_DISABLE_Entity_Foo_float_9

template <class Pybind11T = pybind11::module_ &>
struct Bind_GetNotBoundType : public EntityBase {
  using Pybind11Type = Pybind11T;

  virtual const char *AddFunction() {
    const char *_pb11_weaver_comment_str = nullptr;

#ifndef PB11_WEAVER_DISABLE_GetNotBoundType_AddFunction
    handle.def(
        "GetNotBoundType",
        static_cast<pybind11_weaver::FnPtrT<void, UnexportedType *()>::type>(
            &GetNotBoundType));
#endif
    return _pb11_weaver_comment_str;
  }

  explicit Bind_GetNotBoundType(EntityScope parent_h)
      : handle{static_cast<pybind11::module_ &>(parent_h)} {}

  template <class... HandleArgsT>
  explicit Bind_GetNotBoundType(EntityScope parent_h, HandleArgsT &&...args)
      : handle{std::forward(args)...} {}

  void Update() override { AddFunction(); }

  EntityScope AsScope() override { return EntityScope(handle); }

  static const char *Key() { return "GetNotBoundType"; }

  Pybind11Type handle;
};
#ifndef PB11_WEAVER_DISABLE_Entity_GetNotBoundType

using Entity_GetNotBoundType = Bind_GetNotBoundType<>;

#else

struct Entity_GetNotBoundType : public pybind11_weaver::DisabledEntity {
  explicit Entity_GetNotBoundType(EntityScope parent_h) {}
  static const char *Key() { return "GetNotBoundType"; }
};

#endif // PB11_WEAVER_DISABLE_Entity_GetNotBoundType

template <class Pybind11T = pybind11::module_>
struct Bind_Q : public EntityBase {
  using Pybind11Type = Pybind11T;

  explicit Bind_Q(EntityScope parent_h)
      : handle{static_cast<pybind11::module_ &>(parent_h).def_submodule("Q")} {}

  template <class... HandleArgsT>
  explicit Bind_Q(EntityScope parent_h, HandleArgsT &&...args)
      : handle{std::forward(args)...} {}

  void Update() override {}

  EntityScope AsScope() override { return EntityScope(handle); }

  static const char *Key() { return "Q"; }

  Pybind11Type handle;
};
#ifndef PB11_WEAVER_DISABLE_Entity_Q

using Entity_Q = Bind_Q<>;

#else

struct Entity_Q : public pybind11_weaver::DisabledEntity {
  explicit Entity_Q(EntityScope parent_h) {}
  static const char *Key() { return "Q"; }
};

#endif // PB11_WEAVER_DISABLE_Entity_Q

template <class Pybind11T = pybind11::class_<Q::R<int>>>
struct Bind_Q_R6int9 : public EntityBase {
  using Pybind11Type = Pybind11T;
  using T = int;

  explicit Bind_Q_R6int9(EntityScope parent_h)
      : handle{parent_h, "R6int9", pybind11::dynamic_attr()} {}

  template <class... HandleArgsT>
  explicit Bind_Q_R6int9(EntityScope parent_h, HandleArgsT &&...args)
      : handle{std::forward(args)...} {}

  void Update() override {
    pybind11_weaver::TryAddDefaultCtor<Q::R<int>>(handle);
  }

  EntityScope AsScope() override { return EntityScope(handle); }

  static const char *Key() { return "Q_R6int9"; }

  Pybind11Type handle;
};
#ifndef PB11_WEAVER_DISABLE_Entity_Q_R6int9

using Entity_Q_R6int9 = Bind_Q_R6int9<>;

#else

struct Entity_Q_R6int9 : public pybind11_weaver::DisabledEntity {
  explicit Entity_Q_R6int9(EntityScope parent_h) {}
  static const char *Key() { return "Q_R6int9"; }
};

#endif // PB11_WEAVER_DISABLE_Entity_Q_R6int9

template <class Pybind11T =
              pybind11::class_<template_ns::TemplateClass<double>>>
struct Bind_template_ns_TemplateClass6double9 : public EntityBase {
  using Pybind11Type = Pybind11T;
  using T = double;

  virtual const char *AddMethod_Method() {

    const char *_pb11_weaver_comment_str = nullptr;

#ifndef PB11_WEAVER_DISABLE_template_ns_TemplateClass6double9_Method
    handle.def("Method", static_cast<pybind11_weaver::FnPtrT<
                             template_ns::TemplateClass<T>, T, T *>::type>(
                             &template_ns::TemplateClass<T>::Method));
#endif
    return _pb11_weaver_comment_str;
  }

  explicit Bind_template_ns_TemplateClass6double9(EntityScope parent_h)
      : handle{parent_h, "TemplateClass6double9", pybind11::dynamic_attr()} {}

  template <class... HandleArgsT>
  explicit Bind_template_ns_TemplateClass6double9(EntityScope parent_h,
                                                  HandleArgsT &&...args)
      : handle{std::forward(args)...} {}

  void Update() override {
    AddMethod_Method();
    pybind11_weaver::TryAddDefaultCtor<template_ns::TemplateClass<double>>(
        handle);
  }

  EntityScope AsScope() override { return EntityScope(handle); }

  static const char *Key() { return "template_ns_TemplateClass6double9"; }

  Pybind11Type handle;
};
#ifndef PB11_WEAVER_DISABLE_Entity_template_ns_TemplateClass6double9

using Entity_template_ns_TemplateClass6double9 =
    Bind_template_ns_TemplateClass6double9<>;

#else

struct Entity_template_ns_TemplateClass6double9
    : public pybind11_weaver::DisabledEntity {
  explicit Entity_template_ns_TemplateClass6double9(EntityScope parent_h) {}
  static const char *Key() { return "template_ns_TemplateClass6double9"; }
};

#endif // PB11_WEAVER_DISABLE_Entity_template_ns_TemplateClass6double9

template <class Pybind11T = pybind11::module_ &>
struct Bind_TopFunction : public EntityBase {
  using Pybind11Type = Pybind11T;

  virtual const char *AddFunction() {
    const char *_pb11_weaver_comment_str = R"_pb11_weaver(/**
 * This is Function doc
 * @param i This is i doc
 */)_pb11_weaver";

#ifndef PB11_WEAVER_DISABLE_TopFunction_AddFunction
    handle.def("TopFunction",
               static_cast<pybind11_weaver::FnPtrT<void, void(int)>::type>(
                   &TopFunction),
               _pb11_weaver_comment_str);
#endif
    return _pb11_weaver_comment_str;
  }

  explicit Bind_TopFunction(EntityScope parent_h)
      : handle{static_cast<pybind11::module_ &>(parent_h)} {}

  template <class... HandleArgsT>
  explicit Bind_TopFunction(EntityScope parent_h, HandleArgsT &&...args)
      : handle{std::forward(args)...} {}

  void Update() override { AddFunction(); }

  EntityScope AsScope() override { return EntityScope(handle); }

  static const char *Key() { return "TopFunction"; }

  Pybind11Type handle;
};
#ifndef PB11_WEAVER_DISABLE_Entity_TopFunction

using Entity_TopFunction = Bind_TopFunction<>;

#else

struct Entity_TopFunction : public pybind11_weaver::DisabledEntity {
  explicit Entity_TopFunction(EntityScope parent_h) {}
  static const char *Key() { return "TopFunction"; }
};

#endif // PB11_WEAVER_DISABLE_Entity_TopFunction

template <class Pybind11T = pybind11::module_ &>
struct Bind_TopFunction1 : public EntityBase {
  using Pybind11Type = Pybind11T;

  virtual const char *AddFunction() {
    const char *_pb11_weaver_comment_str = nullptr;

#ifndef PB11_WEAVER_DISABLE_TopFunction1_AddFunction
    handle.def("TopFunction",
               static_cast<pybind11_weaver::FnPtrT<
                   void, void(std::basic_string<char> &)>::type>(&TopFunction));
#endif
    return _pb11_weaver_comment_str;
  }

  explicit Bind_TopFunction1(EntityScope parent_h)
      : handle{static_cast<pybind11::module_ &>(parent_h)} {}

  template <class... HandleArgsT>
  explicit Bind_TopFunction1(EntityScope parent_h, HandleArgsT &&...args)
      : handle{std::forward(args)...} {}

  void Update() override { AddFunction(); }

  EntityScope AsScope() override { return EntityScope(handle); }

  static const char *Key() { return "TopFunction1"; }

  Pybind11Type handle;
};
#ifndef PB11_WEAVER_DISABLE_Entity_TopFunction1

using Entity_TopFunction1 = Bind_TopFunction1<>;

#else

struct Entity_TopFunction1 : public pybind11_weaver::DisabledEntity {
  explicit Entity_TopFunction1(EntityScope parent_h) {}
  static const char *Key() { return "TopFunction1"; }
};

#endif // PB11_WEAVER_DISABLE_Entity_TopFunction1

template <class Pybind11T = pybind11::module_ &>
struct Bind_TopFunctionDef : public EntityBase {
  using Pybind11Type = Pybind11T;

  virtual const char *AddFunction() {
    const char *_pb11_weaver_comment_str = nullptr;

#ifndef PB11_WEAVER_DISABLE_TopFunctionDef_AddFunction
    handle.def("TopFunctionDef",
               static_cast<pybind11_weaver::FnPtrT<void, void()>::type>(
                   &TopFunctionDef));
#endif
    return _pb11_weaver_comment_str;
  }

  explicit Bind_TopFunctionDef(EntityScope parent_h)
      : handle{static_cast<pybind11::module_ &>(parent_h)} {}

  template <class... HandleArgsT>
  explicit Bind_TopFunctionDef(EntityScope parent_h, HandleArgsT &&...args)
      : handle{std::forward(args)...} {}

  void Update() override { AddFunction(); }

  EntityScope AsScope() override { return EntityScope(handle); }

  static const char *Key() { return "TopFunctionDef"; }

  Pybind11Type handle;
};
#ifndef PB11_WEAVER_DISABLE_Entity_TopFunctionDef

using Entity_TopFunctionDef = Bind_TopFunctionDef<>;

#else

struct Entity_TopFunctionDef : public pybind11_weaver::DisabledEntity {
  explicit Entity_TopFunctionDef(EntityScope parent_h) {}
  static const char *Key() { return "TopFunctionDef"; }
};

#endif // PB11_WEAVER_DISABLE_Entity_TopFunctionDef

template <class Pybind11T = pybind11::module_>
struct Bind_disabled_member : public EntityBase {
  using Pybind11Type = Pybind11T;

  explicit Bind_disabled_member(EntityScope parent_h)
      : handle{static_cast<pybind11::module_ &>(parent_h).def_submodule(
            "disabled_member")} {}

  template <class... HandleArgsT>
  explicit Bind_disabled_member(EntityScope parent_h, HandleArgsT &&...args)
      : handle{std::forward(args)...} {}

  void Update() override {}

  EntityScope AsScope() override { return EntityScope(handle); }

  static const char *Key() { return "disabled_member"; }

  Pybind11Type handle;
};
#ifndef PB11_WEAVER_DISABLE_Entity_disabled_member

using Entity_disabled_member = Bind_disabled_member<>;

#else

struct Entity_disabled_member : public pybind11_weaver::DisabledEntity {
  explicit Entity_disabled_member(EntityScope parent_h) {}
  static const char *Key() { return "disabled_member"; }
};

#endif // PB11_WEAVER_DISABLE_Entity_disabled_member

template <class Pybind11T = pybind11::enum_<disabled_member::disabled_Foo>>
struct Bind_disabled_member_disabled_Foo : public EntityBase {
  using Pybind11Type = Pybind11T;

  explicit Bind_disabled_member_disabled_Foo(EntityScope parent_h)
      : handle{parent_h, "disabled_Foo", pybind11::arithmetic()} {}

  template <class... HandleArgsT>
  explicit Bind_disabled_member_disabled_Foo(EntityScope parent_h,
                                             HandleArgsT &&...args)
      : handle{std::forward(args)...} {}

  void Update() override {
    handle.value("BAR", disabled_member::disabled_Foo::BAR);
    handle.value("BAZ", disabled_member::disabled_Foo::BAZ);
  }

  EntityScope AsScope() override { return EntityScope(handle); }

  static const char *Key() { return "disabled_member_disabled_Foo"; }

  Pybind11Type handle;
};
#ifndef PB11_WEAVER_DISABLE_Entity_disabled_member_disabled_Foo

using Entity_disabled_member_disabled_Foo = Bind_disabled_member_disabled_Foo<>;

#else

struct Entity_disabled_member_disabled_Foo
    : public pybind11_weaver::DisabledEntity {
  explicit Entity_disabled_member_disabled_Foo(EntityScope parent_h) {}
  static const char *Key() { return "disabled_member_disabled_Foo"; }
};

#endif // PB11_WEAVER_DISABLE_Entity_disabled_member_disabled_Foo

template <class Pybind11T = pybind11::module_>
struct Bind_disabled_space : public EntityBase {
  using Pybind11Type = Pybind11T;

  explicit Bind_disabled_space(EntityScope parent_h)
      : handle{static_cast<pybind11::module_ &>(parent_h).def_submodule(
            "disabled_space")} {}

  template <class... HandleArgsT>
  explicit Bind_disabled_space(EntityScope parent_h, HandleArgsT &&...args)
      : handle{std::forward(args)...} {}

  void Update() override {}

  EntityScope AsScope() override { return EntityScope(handle); }

  static const char *Key() { return "disabled_space"; }

  Pybind11Type handle;
};
#ifndef PB11_WEAVER_DISABLE_Entity_disabled_space

using Entity_disabled_space = Bind_disabled_space<>;

#else

struct Entity_disabled_space : public pybind11_weaver::DisabledEntity {
  explicit Entity_disabled_space(EntityScope parent_h) {}
  static const char *Key() { return "disabled_space"; }
};

#endif // PB11_WEAVER_DISABLE_Entity_disabled_space

template <class Pybind11T = pybind11::enum_<disabled_space::Foo>>
struct Bind_disabled_space_Foo : public EntityBase {
  using Pybind11Type = Pybind11T;

  explicit Bind_disabled_space_Foo(EntityScope parent_h)
      : handle{parent_h, "Foo", pybind11::arithmetic()} {}

  template <class... HandleArgsT>
  explicit Bind_disabled_space_Foo(EntityScope parent_h, HandleArgsT &&...args)
      : handle{std::forward(args)...} {}

  void Update() override {
    handle.value("BAR", disabled_space::Foo::BAR);
    handle.value("BAZ", disabled_space::Foo::BAZ);
  }

  EntityScope AsScope() override { return EntityScope(handle); }

  static const char *Key() { return "disabled_space_Foo"; }

  Pybind11Type handle;
};
#ifndef PB11_WEAVER_DISABLE_Entity_disabled_space_Foo

using Entity_disabled_space_Foo = Bind_disabled_space_Foo<>;

#else

struct Entity_disabled_space_Foo : public pybind11_weaver::DisabledEntity {
  explicit Entity_disabled_space_Foo(EntityScope parent_h) {}
  static const char *Key() { return "disabled_space_Foo"; }
};

#endif // PB11_WEAVER_DISABLE_Entity_disabled_space_Foo

template <class Pybind11T = pybind11::module_>
struct Bind_earth : public EntityBase {
  using Pybind11Type = Pybind11T;

  explicit Bind_earth(EntityScope parent_h)
      : handle{static_cast<pybind11::module_ &>(parent_h).def_submodule(
            "earth")} {}

  template <class... HandleArgsT>
  explicit Bind_earth(EntityScope parent_h, HandleArgsT &&...args)
      : handle{std::forward(args)...} {}

  void Update() override {}

  EntityScope AsScope() override { return EntityScope(handle); }

  static const char *Key() { return "earth"; }

  Pybind11Type handle;
};
#ifndef PB11_WEAVER_DISABLE_Entity_earth

using Entity_earth = Bind_earth<>;

#else

struct Entity_earth : public pybind11_weaver::DisabledEntity {
  explicit Entity_earth(EntityScope parent_h) {}
  static const char *Key() { return "earth"; }
};

#endif // PB11_WEAVER_DISABLE_Entity_earth

template <class Pybind11T = pybind11::module_>
struct Bind_earth_creatures : public EntityBase {
  using Pybind11Type = Pybind11T;

  explicit Bind_earth_creatures(EntityScope parent_h)
      : handle{static_cast<pybind11::module_ &>(parent_h).def_submodule(
            "creatures")} {}

  template <class... HandleArgsT>
  explicit Bind_earth_creatures(EntityScope parent_h, HandleArgsT &&...args)
      : handle{std::forward(args)...} {}

  void Update() override {}

  EntityScope AsScope() override { return EntityScope(handle); }

  static const char *Key() { return "earth_creatures"; }

  Pybind11Type handle;
};
#ifndef PB11_WEAVER_DISABLE_Entity_earth_creatures

using Entity_earth_creatures = Bind_earth_creatures<>;

#else

struct Entity_earth_creatures : public pybind11_weaver::DisabledEntity {
  explicit Entity_earth_creatures(EntityScope parent_h) {}
  static const char *Key() { return "earth_creatures"; }
};

#endif // PB11_WEAVER_DISABLE_Entity_earth_creatures

template <class Pybind11T = pybind11::enum_<earth::creatures::Animal>>
struct Bind_earth_creatures_Animal : public EntityBase {
  using Pybind11Type = Pybind11T;

  explicit Bind_earth_creatures_Animal(EntityScope parent_h)
      : handle{parent_h, "Animal", pybind11::arithmetic(),
               R"_pb11_weaver(/// This is Enum doc)_pb11_weaver"} {}

  template <class... HandleArgsT>
  explicit Bind_earth_creatures_Animal(EntityScope parent_h,
                                       HandleArgsT &&...args)
      : handle{std::forward(args)...} {}

  void Update() override {
    handle.value("DOG", earth::creatures::Animal::DOG,
                 R"_pb11_weaver(///< This is Enum Item)_pb11_weaver");
    handle.value("CAT", earth::creatures::Animal::CAT,
                 R"_pb11_weaver(///< Cat doc)_pb11_weaver");
  }

  EntityScope AsScope() override { return EntityScope(handle); }

  static const char *Key() { return "earth_creatures_Animal"; }

  Pybind11Type handle;
};
#ifndef PB11_WEAVER_DISABLE_Entity_earth_creatures_Animal

using Entity_earth_creatures_Animal = Bind_earth_creatures_Animal<>;

#else

struct Entity_earth_creatures_Animal : public pybind11_weaver::DisabledEntity {
  explicit Entity_earth_creatures_Animal(EntityScope parent_h) {}
  static const char *Key() { return "earth_creatures_Animal"; }
};

#endif // PB11_WEAVER_DISABLE_Entity_earth_creatures_Animal

template <class Pybind11T = pybind11::class_<earth::creatures::Home>>
struct Bind_earth_creatures_Home : public EntityBase {
  using Pybind11Type = Pybind11T;

  virtual const char *AddMethod_Method() {

    const char *_pb11_weaver_comment_str = nullptr;

#ifndef PB11_WEAVER_DISABLE_earth_creatures_Home_Method
    handle.def(
        "Method",
        static_cast<
            pybind11_weaver::FnPtrT<earth::creatures::Home, double,
                                    std::basic_string<char> &, int>::type>(
            &earth::creatures::Home::Method));
#endif
    return _pb11_weaver_comment_str;
  }

  explicit Bind_earth_creatures_Home(EntityScope parent_h)
      : handle{parent_h, "Home", pybind11::dynamic_attr()} {}

  template <class... HandleArgsT>
  explicit Bind_earth_creatures_Home(EntityScope parent_h,
                                     HandleArgsT &&...args)
      : handle{std::forward(args)...} {}

  void Update() override {
    AddMethod_Method();
    pybind11_weaver::TryAddDefaultCtor<earth::creatures::Home>(handle);
  }

  EntityScope AsScope() override { return EntityScope(handle); }

  static const char *Key() { return "earth_creatures_Home"; }

  Pybind11Type handle;
};
#ifndef PB11_WEAVER_DISABLE_Entity_earth_creatures_Home

using Entity_earth_creatures_Home = Bind_earth_creatures_Home<>;

#else

struct Entity_earth_creatures_Home : public pybind11_weaver::DisabledEntity {
  explicit Entity_earth_creatures_Home(EntityScope parent_h) {}
  static const char *Key() { return "earth_creatures_Home"; }
};

#endif // PB11_WEAVER_DISABLE_Entity_earth_creatures_Home

template <class Pybind11T = pybind11::enum_<earth::creatures::Home::Food>>
struct Bind_earth_creatures_Home_Food : public EntityBase {
  using Pybind11Type = Pybind11T;

  explicit Bind_earth_creatures_Home_Food(EntityScope parent_h)
      : handle{parent_h, "Food", pybind11::arithmetic()} {}

  template <class... HandleArgsT>
  explicit Bind_earth_creatures_Home_Food(EntityScope parent_h,
                                          HandleArgsT &&...args)
      : handle{std::forward(args)...} {}

  void Update() override {
    handle.value("MEAT", earth::creatures::Home::Food::MEAT);
    handle.value("RICE", earth::creatures::Home::Food::RICE);
  }

  EntityScope AsScope() override { return EntityScope(handle); }

  static const char *Key() { return "earth_creatures_Home_Food"; }

  Pybind11Type handle;
};
#ifndef PB11_WEAVER_DISABLE_Entity_earth_creatures_Home_Food

using Entity_earth_creatures_Home_Food = Bind_earth_creatures_Home_Food<>;

#else

struct Entity_earth_creatures_Home_Food
    : public pybind11_weaver::DisabledEntity {
  explicit Entity_earth_creatures_Home_Food(EntityScope parent_h) {}
  static const char *Key() { return "earth_creatures_Home_Food"; }
};

#endif // PB11_WEAVER_DISABLE_Entity_earth_creatures_Home_Food

template <class Pybind11T = pybind11::module_ &>
struct Bind_NSFunction : public EntityBase {
  using Pybind11Type = Pybind11T;

  virtual const char *AddFunction() {
    const char *_pb11_weaver_comment_str = nullptr;

#ifndef PB11_WEAVER_DISABLE_NSFunction_AddFunction
    handle.def("NSFunction",
               static_cast<pybind11_weaver::FnPtrT<
                   void, int(const std::basic_string<char> &)>::type>(
                   &earth::creatures::NSFunction));
#endif
    return _pb11_weaver_comment_str;
  }

  explicit Bind_NSFunction(EntityScope parent_h)
      : handle{static_cast<pybind11::module_ &>(parent_h)} {}

  template <class... HandleArgsT>
  explicit Bind_NSFunction(EntityScope parent_h, HandleArgsT &&...args)
      : handle{std::forward(args)...} {}

  void Update() override { AddFunction(); }

  EntityScope AsScope() override { return EntityScope(handle); }

  static const char *Key() { return "NSFunction"; }

  Pybind11Type handle;
};
#ifndef PB11_WEAVER_DISABLE_Entity_NSFunction

using Entity_NSFunction = Bind_NSFunction<>;

#else

struct Entity_NSFunction : public pybind11_weaver::DisabledEntity {
  explicit Entity_NSFunction(EntityScope parent_h) {}
  static const char *Key() { return "NSFunction"; }
};

#endif // PB11_WEAVER_DISABLE_Entity_NSFunction

template <class Pybind11T = pybind11::module_ &>
struct Bind_NSFunctionDef : public EntityBase {
  using Pybind11Type = Pybind11T;

  virtual const char *AddFunction() {
    const char *_pb11_weaver_comment_str = nullptr;

#ifndef PB11_WEAVER_DISABLE_NSFunctionDef_AddFunction
    handle.def("NSFunctionDef",
               static_cast<pybind11_weaver::FnPtrT<void, void()>::type>(
                   &earth::creatures::NSFunctionDef));
#endif
    return _pb11_weaver_comment_str;
  }

  explicit Bind_NSFunctionDef(EntityScope parent_h)
      : handle{static_cast<pybind11::module_ &>(parent_h)} {}

  template <class... HandleArgsT>
  explicit Bind_NSFunctionDef(EntityScope parent_h, HandleArgsT &&...args)
      : handle{std::forward(args)...} {}

  void Update() override { AddFunction(); }

  EntityScope AsScope() override { return EntityScope(handle); }

  static const char *Key() { return "NSFunctionDef"; }

  Pybind11Type handle;
};
#ifndef PB11_WEAVER_DISABLE_Entity_NSFunctionDef

using Entity_NSFunctionDef = Bind_NSFunctionDef<>;

#else

struct Entity_NSFunctionDef : public pybind11_weaver::DisabledEntity {
  explicit Entity_NSFunctionDef(EntityScope parent_h) {}
  static const char *Key() { return "NSFunctionDef"; }
};

#endif // PB11_WEAVER_DISABLE_Entity_NSFunctionDef

template <class Pybind11T = pybind11::class_<earth::creatures::SweetHome>>
struct Bind_earth_creatures_SweetHome : public EntityBase {
  using Pybind11Type = Pybind11T;

  virtual const char *AddMethod_Method() {

    const char *_pb11_weaver_comment_str = R"_pb11_weaver(/**
   * This is Method doc
   * @return
   */)_pb11_weaver";

#ifndef PB11_WEAVER_DISABLE_earth_creatures_SweetHome_Method
    handle.def("Method",
               static_cast<pybind11_weaver::FnPtrT<earth::creatures::SweetHome,
                                                   int, int>::type>(
                   &earth::creatures::SweetHome::Method),
               _pb11_weaver_comment_str);
#endif
    return _pb11_weaver_comment_str;
  }

  virtual const char *AddMethod_Method1() {

    const char *_pb11_weaver_comment_str = nullptr;

#ifndef PB11_WEAVER_DISABLE_earth_creatures_SweetHome_Method1
    handle.def(
        "Method",
        static_cast<pybind11_weaver::FnPtrT<earth::creatures::SweetHome, void,
                                            std::basic_string<char> &>::type>(
            &earth::creatures::SweetHome::Method));
#endif
    return _pb11_weaver_comment_str;
  }

  virtual const char *AddMethod_VirtualMethod() {

    const char *_pb11_weaver_comment_str = nullptr;

#ifndef PB11_WEAVER_DISABLE_earth_creatures_SweetHome_VirtualMethod
    handle.def("VirtualMethod",
               static_cast<pybind11_weaver::FnPtrT<earth::creatures::SweetHome,
                                                   void, int>::type>(
                   &earth::creatures::SweetHome::VirtualMethod));
#endif
    return _pb11_weaver_comment_str;
  }

  virtual const char *AddMethod_VirtualMethod1() {

    const char *_pb11_weaver_comment_str = nullptr;

#ifndef PB11_WEAVER_DISABLE_earth_creatures_SweetHome_VirtualMethod1
    handle.def(
        "VirtualMethod",
        static_cast<pybind11_weaver::FnPtrT<earth::creatures::SweetHome, void,
                                            std::basic_string<char> &>::type>(
            &earth::creatures::SweetHome::VirtualMethod));
#endif
    return _pb11_weaver_comment_str;
  }

  virtual const char *AddMethod_StaticMethod() {

    const char *_pb11_weaver_comment_str = nullptr;

#ifndef PB11_WEAVER_DISABLE_earth_creatures_SweetHome_StaticMethod
    handle.def_static(
        "StaticMethod",
        static_cast<pybind11_weaver::FnPtrT<void, void(int)>::type>(
            &earth::creatures::SweetHome::StaticMethod));
#endif
    return _pb11_weaver_comment_str;
  }

  virtual const char *AddMethod_StaticMethod1() {

    const char *_pb11_weaver_comment_str = nullptr;

#ifndef PB11_WEAVER_DISABLE_earth_creatures_SweetHome_StaticMethod1
    handle.def_static("StaticMethod",
                      static_cast<pybind11_weaver::FnPtrT<
                          void, void(std::basic_string<char> &)>::type>(
                          &earth::creatures::SweetHome::StaticMethod));
#endif
    return _pb11_weaver_comment_str;
  }

  virtual const char *AddMethod_use_c_callback() {

    const char *_pb11_weaver_comment_str = nullptr;

#ifndef PB11_WEAVER_DISABLE_earth_creatures_SweetHome_use_c_callback
    handle.def(
        "use_c_callback",
        [](earth::creatures::SweetHome &self,
           std::function<int(int, pybind11_weaver::WrappedPtrT<void *>)>
               callback) {
          return std::invoke(
              static_cast<pybind11_weaver::FnPtrT<earth::creatures::SweetHome,
                                                  std::basic_string<char>,
                                                  int (*)(int, void *)>::type>(
                  &earth::creatures::SweetHome::use_c_callback),
              &self,
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
#endif
    return _pb11_weaver_comment_str;
  }

  virtual const char *AddMethod_use_cpp_callback() {

    const char *_pb11_weaver_comment_str = nullptr;

#ifndef PB11_WEAVER_DISABLE_earth_creatures_SweetHome_use_cpp_callback
    handle.def(
        "use_cpp_callback",
        [](earth::creatures::SweetHome &self,
           std::function<int(int, pybind11_weaver::WrappedPtrT<void *>)>
               callback) {
          return std::invoke(
              static_cast<pybind11_weaver::FnPtrT<
                  earth::creatures::SweetHome, std::basic_string<char>,
                  std::function<int(int, void *)>>::type>(
                  &earth::creatures::SweetHome::use_cpp_callback),
              &self,
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
#endif
    return _pb11_weaver_comment_str;
  }

  virtual const char *AddCtor0() {
    const char *_pb11_weaver_comment_str = R"_pb11_weaver(/**
 * This is Class doc
 */)_pb11_weaver";

#ifndef PB11_WEAVER_DISABLE_earth_creatures_SweetHome_Ctor0
    handle.def(pybind11::init<int, float, const std::basic_string<char> &,
                              std::basic_string<char> *>(),
               _pb11_weaver_comment_str);
#endif
    return _pb11_weaver_comment_str;
  }

  explicit Bind_earth_creatures_SweetHome(EntityScope parent_h)
      : handle{parent_h, "SweetHome", pybind11::dynamic_attr(),
               R"_pb11_weaver(/**
 * This is Class doc
 */)_pb11_weaver"} {}

  template <class... HandleArgsT>
  explicit Bind_earth_creatures_SweetHome(EntityScope parent_h,
                                          HandleArgsT &&...args)
      : handle{std::forward(args)...} {}

  void Update() override {
    AddMethod_Method();
    AddMethod_Method1();
    AddMethod_VirtualMethod();
    AddMethod_VirtualMethod1();
    AddMethod_StaticMethod();
    AddMethod_StaticMethod1();
    AddMethod_use_c_callback();
    AddMethod_use_cpp_callback();
    AddCtor0();
    handle.def_readwrite("member", &earth::creatures::SweetHome::member,
                         R"_pb11_weaver(///< This is Member doc)_pb11_weaver");
  }

  EntityScope AsScope() override { return EntityScope(handle); }

  static const char *Key() { return "earth_creatures_SweetHome"; }

  Pybind11Type handle;
};
#ifndef PB11_WEAVER_DISABLE_Entity_earth_creatures_SweetHome

using Entity_earth_creatures_SweetHome = Bind_earth_creatures_SweetHome<>;

#else

struct Entity_earth_creatures_SweetHome
    : public pybind11_weaver::DisabledEntity {
  explicit Entity_earth_creatures_SweetHome(EntityScope parent_h) {}
  static const char *Key() { return "earth_creatures_SweetHome"; }
};

#endif // PB11_WEAVER_DISABLE_Entity_earth_creatures_SweetHome

template <class Pybind11T = pybind11::enum_<earth::creatures::ValueSet>>
struct Bind_earth_creatures_ValueSet : public EntityBase {
  using Pybind11Type = Pybind11T;

  explicit Bind_earth_creatures_ValueSet(EntityScope parent_h)
      : handle{parent_h, "ValueSet", pybind11::arithmetic()} {}

  template <class... HandleArgsT>
  explicit Bind_earth_creatures_ValueSet(EntityScope parent_h,
                                         HandleArgsT &&...args)
      : handle{std::forward(args)...} {}

  void Update() override {
    handle.value("LOW", earth::creatures::ValueSet::LOW);
    handle.value("MIDDLE", earth::creatures::ValueSet::MIDDLE);
    handle.value("HIGH", earth::creatures::ValueSet::HIGH);
  }

  EntityScope AsScope() override { return EntityScope(handle); }

  static const char *Key() { return "earth_creatures_ValueSet"; }

  Pybind11Type handle;
};
#ifndef PB11_WEAVER_DISABLE_Entity_earth_creatures_ValueSet

using Entity_earth_creatures_ValueSet = Bind_earth_creatures_ValueSet<>;

#else

struct Entity_earth_creatures_ValueSet
    : public pybind11_weaver::DisabledEntity {
  explicit Entity_earth_creatures_ValueSet(EntityScope parent_h) {}
  static const char *Key() { return "earth_creatures_ValueSet"; }
};

#endif // PB11_WEAVER_DISABLE_Entity_earth_creatures_ValueSet

template <class Pybind11T = pybind11::module_>
struct Bind_template_ns : public EntityBase {
  using Pybind11Type = Pybind11T;

  explicit Bind_template_ns(EntityScope parent_h)
      : handle{static_cast<pybind11::module_ &>(parent_h).def_submodule(
            "template_ns")} {}

  template <class... HandleArgsT>
  explicit Bind_template_ns(EntityScope parent_h, HandleArgsT &&...args)
      : handle{std::forward(args)...} {}

  void Update() override {}

  EntityScope AsScope() override { return EntityScope(handle); }

  static const char *Key() { return "template_ns"; }

  Pybind11Type handle;
};
#ifndef PB11_WEAVER_DISABLE_Entity_template_ns

using Entity_template_ns = Bind_template_ns<>;

#else

struct Entity_template_ns : public pybind11_weaver::DisabledEntity {
  explicit Entity_template_ns(EntityScope parent_h) {}
  static const char *Key() { return "template_ns"; }
};

#endif // PB11_WEAVER_DISABLE_Entity_template_ns

template <class Pybind11T = pybind11::class_<
              template_ns::DerivedClass, template_ns::TemplateClass<double>>>
struct Bind_template_ns_DerivedClass : public EntityBase {
  using Pybind11Type = Pybind11T;

  virtual const char *AddMethod_Derived() {

    const char *_pb11_weaver_comment_str = nullptr;

#ifndef PB11_WEAVER_DISABLE_template_ns_DerivedClass_Derived
    handle.def(
        "Derived",
        static_cast<
            pybind11_weaver::FnPtrT<template_ns::DerivedClass, double>::type>(
            &template_ns::DerivedClass::Derived));
#endif
    return _pb11_weaver_comment_str;
  }

  explicit Bind_template_ns_DerivedClass(EntityScope parent_h)
      : handle{parent_h, "DerivedClass", pybind11::dynamic_attr()} {}

  template <class... HandleArgsT>
  explicit Bind_template_ns_DerivedClass(EntityScope parent_h,
                                         HandleArgsT &&...args)
      : handle{std::forward(args)...} {}

  void Update() override {
    AddMethod_Derived();
    pybind11_weaver::TryAddDefaultCtor<template_ns::DerivedClass>(handle);
  }

  EntityScope AsScope() override { return EntityScope(handle); }

  static const char *Key() { return "template_ns_DerivedClass"; }

  Pybind11Type handle;
};
#ifndef PB11_WEAVER_DISABLE_Entity_template_ns_DerivedClass

using Entity_template_ns_DerivedClass = Bind_template_ns_DerivedClass<>;

#else

struct Entity_template_ns_DerivedClass
    : public pybind11_weaver::DisabledEntity {
  explicit Entity_template_ns_DerivedClass(EntityScope parent_h) {}
  static const char *Key() { return "template_ns_DerivedClass"; }
};

#endif // PB11_WEAVER_DISABLE_Entity_template_ns_DerivedClass

template <class Pybind11T = pybind11::class_<template_ns::TemplateClass<float>>>
struct Bind_template_ns_TemplateClass6float9 : public EntityBase {
  using Pybind11Type = Pybind11T;
  using T = float;

  virtual const char *AddMethod_Get() {

    const char *_pb11_weaver_comment_str = nullptr;

#ifndef PB11_WEAVER_DISABLE_template_ns_TemplateClass6float9_Get
    handle.def("Get", static_cast<pybind11_weaver::FnPtrT<
                          template_ns::TemplateClass<float>, float>::type>(
                          &template_ns::TemplateClass<float>::Get));
#endif
    return _pb11_weaver_comment_str;
  }

  explicit Bind_template_ns_TemplateClass6float9(EntityScope parent_h)
      : handle{parent_h, "TemplateClass6float9", pybind11::dynamic_attr(),
               R"_pb11_weaver(// Specialization is supported directly
// extra binding could be enabled by using extern explicit instantiation)_pb11_weaver"} {
  }

  template <class... HandleArgsT>
  explicit Bind_template_ns_TemplateClass6float9(EntityScope parent_h,
                                                 HandleArgsT &&...args)
      : handle{std::forward(args)...} {}

  void Update() override {
    AddMethod_Get();
    pybind11_weaver::TryAddDefaultCtor<template_ns::TemplateClass<float>>(
        handle);
  }

  EntityScope AsScope() override { return EntityScope(handle); }

  static const char *Key() { return "template_ns_TemplateClass6float9"; }

  Pybind11Type handle;
};
#ifndef PB11_WEAVER_DISABLE_Entity_template_ns_TemplateClass6float9

using Entity_template_ns_TemplateClass6float9 =
    Bind_template_ns_TemplateClass6float9<>;

#else

struct Entity_template_ns_TemplateClass6float9
    : public pybind11_weaver::DisabledEntity {
  explicit Entity_template_ns_TemplateClass6float9(EntityScope parent_h) {}
  static const char *Key() { return "template_ns_TemplateClass6float9"; }
};

#endif // PB11_WEAVER_DISABLE_Entity_template_ns_TemplateClass6float9

template <class Pybind11T = pybind11::class_<template_ns::TemplateClass<int>>>
struct Bind_template_ns_TemplateClass6int9 : public EntityBase {
  using Pybind11Type = Pybind11T;
  using T = int;

  virtual const char *AddMethod_Method() {

    const char *_pb11_weaver_comment_str = nullptr;

#ifndef PB11_WEAVER_DISABLE_template_ns_TemplateClass6int9_Method
    handle.def("Method", static_cast<pybind11_weaver::FnPtrT<
                             template_ns::TemplateClass<T>, T, T *>::type>(
                             &template_ns::TemplateClass<T>::Method));
#endif
    return _pb11_weaver_comment_str;
  }

  explicit Bind_template_ns_TemplateClass6int9(EntityScope parent_h)
      : handle{parent_h, "TemplateClass6int9", pybind11::dynamic_attr(),
               R"_pb11_weaver(// For template class, an extern explicit instantiation is enough.)_pb11_weaver"} {
  }

  template <class... HandleArgsT>
  explicit Bind_template_ns_TemplateClass6int9(EntityScope parent_h,
                                               HandleArgsT &&...args)
      : handle{std::forward(args)...} {}

  void Update() override {
    AddMethod_Method();
    pybind11_weaver::TryAddDefaultCtor<template_ns::TemplateClass<int>>(handle);
  }

  EntityScope AsScope() override { return EntityScope(handle); }

  static const char *Key() { return "template_ns_TemplateClass6int9"; }

  Pybind11Type handle;
};
#ifndef PB11_WEAVER_DISABLE_Entity_template_ns_TemplateClass6int9

using Entity_template_ns_TemplateClass6int9 =
    Bind_template_ns_TemplateClass6int9<>;

#else

struct Entity_template_ns_TemplateClass6int9
    : public pybind11_weaver::DisabledEntity {
  explicit Entity_template_ns_TemplateClass6int9(EntityScope parent_h) {}
  static const char *Key() { return "template_ns_TemplateClass6int9"; }
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
