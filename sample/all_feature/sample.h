#ifndef PYBIND11_WEAVER_SAMPLE_H
#define PYBIND11_WEAVER_SAMPLE_H

#include <functional>
#include <string>

#include "un_export.h"
/**
 * This is Function doc
 * @param i This is i doc
 */
void TopFunction(int);
void TopFunction(std::string &);
inline void TopFunctionDef() {}
namespace earth::creatures {

int NSFunction(const std::string &str);
inline void NSFunctionDef() {}
/// This is Enum doc
enum Animal {
  DOG, ///< This is Enum Item
  CAT, ///< Cat doc
};

enum class ValueSet {
  LOW = 100,
  MIDDLE = 1000,
  HIGH = 10000,
};
struct Home {
  enum class Food {
    MEAT,
    RICE,
  };
  double Method(std::string &, int);
};

/**
 * This is Class doc
 */
class SweetHome {
public:
  SweetHome(int, float, const std::string &, std::string *){};

  /**
   * This is Method doc
   * @return
   */
  int Method(int);
  void Method(std::string &);
  virtual void VirtualMethod(int) {}
  virtual void VirtualMethod(std::string &) {}
  __attribute__((visibility("hidden"))) void HiddenMethod(int);
  static void StaticMethod(int);
  static void StaticMethod(std::string &);
  int member; ///< This is Member doc

  std::string use_c_callback(int (*callback)(int, void *));

  std::string use_cpp_callback(std::function<int(int, void *)> callback);

private:
  SweetHome() = default;
  void PrivateMethod(int);
  static void PrivateStaticMethod(int);
  int private_member;
};
} // namespace earth::creatures

namespace disabled_space {
enum Foo {
  BAR,
  BAZ,
};
}

namespace disabled_member {
enum class disabled_Foo {
  BAR,
  BAZ,
};
}

class Foo; // forward declaration will be ignored
__attribute__((visibility("hidden"))) void
HiddenTopFunction(int); // visibility will be used

UnexportedType *GetNotBoundType();

namespace Q {
template <class T> struct R {};
} // namespace Q

/**
 * This is Function doc
 *
 * Template function is also supported, but need some extra work to declare
 * which instance should be bound.
 * 1. need a helper file to declare functions to bind, like
 * "sample/all_feature/template_pb11_weaver_helper.h"
 * 2. some explicit instantiation or specialization must be provided in the
 *   translation unit.
 */
template <class T, int N> std::string Foo(T, int) { return "Default one"; }

namespace template_ns {

template <class T> struct TemplateClass {
public:
  T Method(T *p);
  T member;
};

template <class T> T TemplateClass<T>::Method(T *p) { return *p; }

// Specialization is supported directly
// extra binding could be enabled by using extern explicit instantiation
template <> class TemplateClass<float> {
public:
  float Get() { return 0; }
};

class DerivedClass : public TemplateClass<double> {
public:
  double Derived() { return 0; }
};
} // namespace template_ns

template <class T> class VirtualBase {
public:
  virtual T foo(std::string a) = 0;
  virtual T inherit_to_final() = 0;
  virtual T inherit_to_private() = 0;
  T call_foo() { return foo("996"); }
};

class DriveVirtual : public VirtualBase<int> {
public:
  int foo(std::string a) override { return 0; }
  int inherit_to_final() final { return 0; }
  virtual float bar(int) { return 1.0; }
  float call_bar() { return bar(996); }

private:
  int inherit_to_private() override { return 0; }
};
#endif // PYBIND11_WEAVER_SAMPLE_H
