import unittest

from pybind11_weaver import gen_unit


class TuLoaderTest(unittest.TestCase):

    def test_get_default_include_flags(self):
        for compiler in ["g++", None]:
            flags = gen_unit.get_default_include_flags(compiler)
            self.assertGreater(len(flags), 0)
            for v in flags:
                self.assertTrue(v.startswith("-I"))

    def test_load_config(self):
        cfg = """
common_config:
  compiler: null
  cxx_flags: [ "a","b" ]
  include_directories: [ "/path/to/foo","/path/to/bar" ]
  output_namespace: "foo::bar"
  root_module_namespace: "gi::jo"

io_configs:
    - inputs: ["a.h"]
      output: "/path/to/output"
      output_namespace: "bar::q"
      extra_cxx_flags: ["c","d"] 
"""
        cfg = gen_unit.load_config(cfg)
        self.assertIsNone(cfg["common_config"]["compiler"])
        self.assertEqual(cfg["common_config"]["cxx_flags"], ["a", "b"])
        self.assertEqual(cfg["common_config"]["include_directories"], ["/path/to/foo", "/path/to/bar"])
        self.assertEqual(cfg["common_config"]["output_namespace"], "foo::bar")
        self.assertEqual(cfg["common_config"]["root_module_namespace"], "gi::jo")
        self.assertEqual(cfg["io_configs"][0]["inputs"], ["a.h"])
        self.assertEqual(cfg["io_configs"][0]["output"], "/path/to/output")
        self.assertEqual(cfg["io_configs"][0]["output_namespace"], "bar::q")
        self.assertEqual(cfg["io_configs"][0]["extra_cxx_flags"], ["c", "d"])

    def test_load_default_config(self):
        cfg = gen_unit.load_config("")
        self.assertIsNone(cfg["common_config"]["compiler"])
        self.assertEqual(cfg["common_config"]["cxx_flags"], [])
        self.assertEqual(cfg["common_config"]["include_directories"], [])
        self.assertEqual(cfg["common_config"]["output_namespace"], "")
        self.assertEqual(cfg["common_config"]["root_module_namespace"], "")
        self.assertEqual(len(cfg["io_configs"]), 0)
        cfg = gen_unit.load_config("""
io_configs:
    - inputs: ["a.h"]
      output: "/path/to/output0"
    - inputs: ["b.h"]
      output: "/path/to/output1"
""")
        self.assertEqual(cfg["io_configs"][0]["inputs"], ["a.h"])
        self.assertEqual(cfg["io_configs"][0]["output"], "/path/to/output0")
        self.assertEqual(cfg["io_configs"][0]["extra_cxx_flags"], [])
        self.assertIsNone(cfg["io_configs"][0]["output_namespace"])
        self.assertEqual(cfg["io_configs"][1]["inputs"], ["b.h"])
        self.assertEqual(cfg["io_configs"][1]["output"], "/path/to/output1")
        self.assertEqual(cfg["io_configs"][1]["extra_cxx_flags"], [])
        self.assertIsNone(cfg["io_configs"][1]["output_namespace"])

    def test_gen_unit_load(self):
        gen_units = gen_unit.load_gen_unit_from_config("""
io_configs:
    - inputs: [<iostream>]
      output: "/path/to/output"
    - inputs: [<cstdio>]
      output: "/path/to/output2"
"""
                                                       )
        self.assertEqual(len(gen_units), 2)


if __name__ == "__main__":
    unittest.main()
