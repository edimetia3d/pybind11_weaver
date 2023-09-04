
#include <pybind11/pybind11.h>

#include "sample_binding.cc.inc"

// When you want to customize the binding of `XXXX`, you have 2 options:
// a. Inherit the template class `Bind_XXXX<>` and implement
//   all the virtual functions. By doing so, you have full control of the
//   binding.
// b. Inherit the class `Entity_XXXX` and override some virtual functions.
//  By doing so, you can not change the type and the construction of the
//  pybind11 object, but you can do any other things.

// In this example, we will use option b to
// 1. add a new method to the python class
// 2. change the binding of a method
namespace {
class CustomSweetHome : public Entity_earth_creatures_SweetHome {
public:
  using Entity_earth_creatures_SweetHome::Entity_earth_creatures_SweetHome;

  void Update() override {
    Entity_earth_creatures_SweetHome::Update();

    // We can add a new method in any virtual function.
    // but add it here makes the code more readable.
    handle.def("new_method",
               [](earth::creatures::SweetHome &self) { return 1; });
  }

  void BindMethod_Method(Pybind11Type &obj) override {
    obj.def("Method", [](earth::creatures::SweetHome &self, int i) {
      return self.Method(i + 1);
    });
    obj.def("Method",
            static_cast<void (earth::creatures::SweetHome::*)(std::string &)>(
                &earth::creatures::SweetHome::Method));
  }
};
} // namespace
PYBIND11_MODULE(all_feature_module, m) {
  pybind11_weaver::CustomBindingRegistry reg;
  reg.DisableBinding<Entity_disabled_space>();
  reg.DisableBinding<Entity_disabled_member_disabled_Foo>();
  reg.SetCustom<CustomSweetHome>();
  auto update_guard = DeclFn(m, reg);
}