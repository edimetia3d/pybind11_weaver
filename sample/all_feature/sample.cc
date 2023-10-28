//
// License: MIT
//
#include "sample.h"

void TopFunction(int) {}
void TopFunction(std::string &) {}
void HiddenTopFunction(int) {}
UnexportedType *GetNotBoundType() { return nullptr; }
double earth::creatures::Home::Method(std::string &, int) { return 0; }
int earth::creatures::SweetHome::Method(int i) { return i; }
void earth::creatures::SweetHome::Method(std::string &) {}
void earth::creatures::SweetHome::StaticMethod(int) {}
void earth::creatures::SweetHome::StaticMethod(std::string &) {}
void earth::creatures::SweetHome::PrivateMethod(int) {}
void earth::creatures::SweetHome::PrivateStaticMethod(int) {}
void earth::creatures::SweetHome::HiddenMethod(int) {}
std::string
earth::creatures::SweetHome::use_c_callback(int (*callback)(int, void *)) {
  void *p = reinterpret_cast<void *>(1);
  return std::string("From C Pointer") + std::to_string(callback(1, p));
}

std::string earth::creatures::SweetHome::use_cpp_callback(
    std::function<int(int, void *)> callback) {

  void *p = reinterpret_cast<void *>(2);
  return std::string("From C++ Function") + std::to_string(callback(2, p));
}
int earth::creatures::NSFunction(const std::string &str) { return 0; }

// Specialization of template function, so it could be found by the linker.
template <> std::string Foo<Q::R<int>, 8>(Q::R<int>, int) {
  return "Special one";
}

// Explicit instantiation of template function, so it could be found by the
// linker.
template std::string Foo<float, 9>(float, int);
template class template_ns::TemplateClass<int>;