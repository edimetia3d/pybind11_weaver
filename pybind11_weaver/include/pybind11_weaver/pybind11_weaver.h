#ifndef GITHUB_COM_PYBIND11_WEAVER
#define GITHUB_COM_PYBIND11_WEAVER
#include <atomic>
#include <functional>
#include <map>
#include <mutex>
#include <utility>

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace pybind11_weaver {

template <class T> struct PointerWrapper {
  static_assert(std::is_pointer<T>::value, "T must be a pointer type");
  T ptr;
  PointerWrapper(T ptr) : ptr(ptr) {}
  PointerWrapper(intptr_t ptr_v) : ptr(reinterpret_cast<T>(ptr_v)) {}
  operator T() { return ptr; }
  static void FastBind(pybind11::module &m, const std::string &name) {
    pybind11::class_<PointerWrapper> handle(m, name.c_str(),
                                            pybind11::dynamic_attr());
    handle.def(pybind11::init<intptr_t>());
    handle.def("get_ptr", [](PointerWrapper &self) {
      return reinterpret_cast<intptr_t>(self.ptr);
    });
    handle.def("set_ptr", [](PointerWrapper &self, intptr_t ptr) {
      self.ptr = reinterpret_cast<T>(ptr);
    });
    handle.def_static("from_capsule", [](pybind11::capsule o) {
      return new PointerWrapper<T>(reinterpret_cast<T>(o.get_pointer()));
    });
  }
};
template <class T> using WrappedPtrT = std::unique_ptr<PointerWrapper<T>>;

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
    using CFnT = CR(CArgs...);
    static CFnPtrT Run(std::function<CppFnT> to_call, Guardian &&guard,
                       CFnPtrT c_wrapper, int64_t uuid) {
      // lock
      GetMutex(uuid).lock();
      guard.dtor_callbacks.push_back([uuid]() {
        FnMap().erase(uuid);
        GetMutex(uuid).unlock();
      });
      FnProxy(uuid) = to_call;
      return c_wrapper;
    }
  };

  static std::mutex &GetMutex(int64_t uuid) {
    static std::map<int64_t, std::mutex> mtx;
    return mtx[uuid];
  };

  static std::function<CppFnT> &FnProxy(int64_t uuid) { return FnMap()[uuid]; }
  static std::map<int64_t, std::function<CppFnT>> &FnMap() {
    static std::map<int64_t, std::function<CppFnT>> fns;
    return fns;
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
