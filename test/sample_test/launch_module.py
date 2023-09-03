print("hello")
import all_feature_module

all_feature_module.TopFunction(0)
all_feature_module.TopFunction("str")
all_feature_module.TopFunctionDef()
all_feature_module.earth.creatures.NSFunction("str")
all_feature_module.earth.creatures.NSFunctionDef()

from all_feature_module.earth import creatures

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
assert not hasattr(sweet_home, "PrivateMethod")
assert not hasattr(sweet_home, "private_member")
