import unittest
import random

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

    def _test_callback(self, callback_attr_name, call_back_id):
        captured = []
        randint = random.randint(0, 100)

        def callback(x, ptr):
            # note it's usually not safe to capture a value in callback function.
            # you must make sure the value keeps alive after the callback function returns.
            captured.append(x)
            captured.append(ptr)
            assert call_back_id == x
            assert call_back_id == ptr.get_ptr()
            return randint + x + 1

        sweet_home = creatures.SweetHome(1, 1.0, "str", "str")
        call_cb = getattr(sweet_home, callback_attr_name)
        assert f"{randint + call_back_id + 1}" in call_cb(callback)
        assert len(captured) == 2
        assert captured[0] == call_back_id
        assert isinstance(captured[1], m.voidp)
        assert captured[1].get_ptr() == call_back_id

    def test_callback(self):
        self._test_callback("use_c_callback", 1)
        self._test_callback("use_cpp_callback", 2)

    def test_template_function(self):
        assert "Special one" == m.Foo_Q_R6int9_8(m.R6int9(), 1)
        assert "Default one" == m.Foo_float_9(1.0, 1)

    def test_virtual_trampoline(self):
        class PythonType(m.DriveVirtual):
            def __init__(self):
                super().__init__()

            def foo(self, x):
                assert x == "996"
                return 9996

            def bar(self, y):
                assert y == 996
                return 1996.0

        normal_obj = m.DriveVirtual()
        assert normal_obj.call_foo() == 0
        assert normal_obj.call_bar() == 1.0

        python_obj = PythonType()
        assert python_obj.call_foo() == 9996
        assert python_obj.call_bar() == 1996.0


if __name__ == "__main__":
    unittest.main()
