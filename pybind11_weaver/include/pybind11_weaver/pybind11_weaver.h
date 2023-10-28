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
