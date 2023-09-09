import unittest
import all_feature_module as m
from all_feature_module.earth import creatures


class TestAll(unittest.TestCase):
    def test_all_exported(self):
        m.TopFunction(0)
        m.TopFunction("str")
        m.TopFunctionDef()
        m.earth.creatures.NSFunction("str")
        m.earth.creatures.NSFunctionDef()
        creatures.Animal.DOG
        creatures.Animal.CAT
        assert int(creatures.ValueSet.LOW) == 100
        assert int(creatures.ValueSet.MIDDLE) == 1000
        assert int(creatures.ValueSet.HIGH) == 10000

        creatures.Home.Food.MEAT
        creatures.Home.Food.RICE
        home = creatures.Home()
        home.Method("str", 0)

        sweet_home = creatures.SweetHome(1, 1.0, "str", "str")
        sweet_home.Method(0)
        sweet_home.Method("str")
        sweet_home.StaticMethod(0)
        sweet_home.VirtualMethod(0)
        sweet_home.VirtualMethod("str")
        creatures.SweetHome.StaticMethod(0)
        sweet_home.StaticMethod("str")
        creatures.SweetHome.StaticMethod("str")
        sweet_home.member = 996
        assert sweet_home.member == 996

    def test_access_specifier(self):
        sweet_home = creatures.SweetHome(1, 1.0, "str", "str")
        assert not hasattr(sweet_home, "PrivateMethod")
        assert not hasattr(sweet_home, "private_member")

    def test_custom_disabled_binding(self):
        assert not hasattr(m, "disabled_space")  # disabled by pybind11_weaver
        assert hasattr(m, "disabled_member")
        assert not hasattr(m.disabled_member, "disabled_Foo")

    def test_custom_binding(self):
        sweet_home = creatures.SweetHome(1, 1.0, "str", "str")
        assert hasattr(sweet_home, "new_method")
        assert sweet_home.new_method() == 1
        assert sweet_home.Method(2) == 3

    def test_cpp_visibility_control(self):
        sweet_home = creatures.SweetHome(1, 1.0, "str", "str")
        assert not hasattr(sweet_home, "HiddenMethod")
        assert not hasattr(m, "Foo")
        assert not hasattr(m, "HiddenTopFunction")

    def test_docstring(self):
        assert "This is Function doc" in m.TopFunction.__doc__
        assert "This is Enum doc" in creatures.Animal.__doc__
        assert "This is Enum Item" in creatures.Animal.DOG.__doc__
        assert "This is Class doc" in creatures.SweetHome.__doc__
        # assert "This is Method doc" in creatures.SweetHome.Method.__doc__ // fixme: pybind11 seems not support doc for overloaded function?
        assert " This is Member doc" in creatures.SweetHome.member.__doc__


if __name__ == "__main__":
    unittest.main()
