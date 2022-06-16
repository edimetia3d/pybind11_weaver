import unittest

from pybind11_weaver import gu_loader


class TuLoaderTest(unittest.TestCase):

    def test_get_default_include_flags(self):
        for compiler in ["g++", None]:
            flags = gu_loader.get_default_include_flags(compiler)
            self.assertGreater(len(flags), 0)
            for v in flags:
                self.assertTrue(v.startswith("-I"))

    def test_load_config(self):
        cfg = """
common_config:
  compiler: null
  cxx_flags: [ "a","b" ]
  include_directories: [ "/path/to/foo","/path/to/bar" ]
  namespace: "foo::bar"

io_configs:
    - inputs: ["a.h"]
      output: "/path/to/output"
      namespace: "bar::q"
      extra_cxx_flags: ["c","d"] 
"""
        cfg = gu_loader.load_config(cfg)
        self.assertIsNone(cfg["common_config"]["compiler"])
        self.assertEqual(cfg["common_config"]["cxx_flags"], ["a", "b"])
        self.assertEqual(cfg["common_config"]["include_directories"], ["/path/to/foo", "/path/to/bar"])
        self.assertEqual(cfg["common_config"]["namespace"], "foo::bar")
        self.assertEqual(cfg["io_configs"][0]["inputs"], ["a.h"])
        self.assertEqual(cfg["io_configs"][0]["output"], "/path/to/output")
        self.assertEqual(cfg["io_configs"][0]["namespace"], "bar::q")
        self.assertEqual(cfg["io_configs"][0]["extra_cxx_flags"], ["c", "d"])

    def test_load_default_config(self):
        cfg = gu_loader.load_config("")
        self.assertIsNone(cfg["common_config"]["compiler"])
        self.assertEqual(cfg["common_config"]["cxx_flags"], [])
        self.assertEqual(cfg["common_config"]["include_directories"], [])
        self.assertEqual(cfg["common_config"]["namespace"], "")
        self.assertEqual(len(cfg["io_configs"]), 0)
        cfg = gu_loader.load_config("""
io_configs:
    - inputs: ["a.h"]
      output: "/path/to/output0"
    - inputs: ["b.h"]
      output: "/path/to/output1"
""")
        self.assertEqual(cfg["io_configs"][0]["inputs"], ["a.h"])
        self.assertEqual(cfg["io_configs"][0]["output"], "/path/to/output0")
        self.assertEqual(cfg["io_configs"][0]["extra_cxx_flags"], [])
        self.assertEqual(cfg["io_configs"][0]["namespace"], "")
        self.assertEqual(cfg["io_configs"][1]["inputs"], ["b.h"])
        self.assertEqual(cfg["io_configs"][1]["output"], "/path/to/output1")
        self.assertEqual(cfg["io_configs"][1]["extra_cxx_flags"], [])
        self.assertEqual(cfg["io_configs"][1]["namespace"], "")

    def test_gen_unit_load(self):
        gen_units = gu_loader.load_gen_unit_from_config("""
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
