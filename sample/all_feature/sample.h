#ifndef PYBIND11_WEAVER_SAMPLE_H
#define PYBIND11_WEAVER_SAMPLE_H

#include <string>

void TopFunction(int);
void TopFunction(std::string &);
inline void TopFunctionDef() {}
namespace earth::creatures {

int NSFunction(const std::string &str);
inline void NSFunctionDef() {}
/// This is Animal doc
enum Animal {
  DOG, ///< Dog doc
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
class SweetHome {
public:
  SweetHome(int, float, const std::string &, std::string *){};
  int Method(int);
  void Method(std::string &);
  virtual void VirtualMethod(int) {}
  virtual void VirtualMethod(std::string &) {}
  __attribute__((visibility("hidden"))) void HiddenMethod(int);
  static void StaticMethod(int);
  static void StaticMethod(std::string &);
  int member;

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

#endif // PYBIND11_WEAVER_SAMPLE_H
