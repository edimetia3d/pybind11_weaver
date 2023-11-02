import os
import unittest

from pybind11_weaver import config


class ConfigTest(unittest.TestCase):

    def test_empty_config(self):
        with self.assertRaises(ValueError) as e:
            config.MainConfig.load("")
        self.assertEqual(str(e.exception), "file_or_content can not be empty")

        with self.assertRaises(ValueError) as e:
            config.MainConfig.load("common:")
        self.assertEqual(str(e.exception), "No IOConfig is specified")

        with self.assertRaises(ValueError) as e:
            config.MainConfig.load("""
io_configs: 
            """)
        self.assertEqual(str(e.exception), "Expect 'MainConfig.io_configs' to be a list, but got None")

        with self.assertRaises(ValueError) as e:
            config.MainConfig.load("""
io_configs:
    - inputs: ["path/to/input0"]
            """)
        self.assertEqual(str(e.exception), "Inputs and output can not be empty")

        with self.assertRaises(ValueError) as e:
            config.MainConfig.load("""
io_configs:
    - inputs: []
            """)
        self.assertEqual(str(e.exception), "Inputs and output can not be empty")

    def test_default_config(self):
        cfg = config.MainConfig.load("""
io_configs:
    - inputs: ["a.h"]
      output: "/path/to/output0"
""")

        if cfg.common_config.compiler is not None:
            self.assertTrue(len(cfg.common_config.cxx_flags) > 0)
            self.assertTrue(len(cfg.io_configs[0]._cxx_flags) > 0)
        else:
            self.assertEqual(len(cfg.common_config.compiler), 0)
        self.assertEqual(len(cfg.common_config.include_directories), 0)
        self.assertEqual(len(cfg.io_configs), 1)
        io_cfg = cfg.io_configs[0]
        self.assertEqual(io_cfg._cxx_flags, cfg.common_config.cxx_flags)
        self.assertEqual(io_cfg.decl_fn_name, "DeclFn")
        self.assertEqual(io_cfg.inputs, ['"a.h"'])
        self.assertEqual(io_cfg.output, "/path/to/output0")
        self.assertEqual(io_cfg.root_module_namespace, "")
        self.assertEqual(io_cfg.strict_visibility_mode, False)
        self.assertEqual(io_cfg.gen_docstring, True)
        self.assertEqual(io_cfg.extra_cxx_flags, [])

    def test_load_config_with_docstring(self):
        dir = os.path.dirname(__file__)
        cfg = config.MainConfig.load(os.path.join(dir, "config_with_doc.yaml"))
        self.assertIsNotNone(cfg)

    def test_common_cfg_cxx_flag_normalize(self):
        cfg = config.MainConfig.load("""
common_config:
    cxx_flags: ["-std=c++foo -I/path/to/include/bar"]
io_configs:
    - inputs: ["a.h"]
      output: "/path/to/output0"
""")
        expect_vs = ["-std=c++foo", "-I/path/to/include/bar"]
        self.assertGreaterEqual(len(cfg.common_config.cxx_flags), 2)
        for v in expect_vs:
            self.assertIn(v, cfg.common_config.cxx_flags)
            self.assertIn(v, cfg.io_configs[0]._cxx_flags)

    def test_glob_and_relative_path(self):
        cur_dir = os.path.dirname(__file__)
        file_name = os.path.basename(__file__)
        cfg = config.MainConfig.load(f"""
common_config:
    include_directories: ["{cur_dir}"]
io_configs:
    - inputs: ["glob('{cur_dir}/*.py')"]
      output: "/path/to/output0"
""")
        self.assertEqual(cfg.io_configs[0].inputs, [f'"{file_name}"'])


if __name__ == "__main__":
    unittest.main()
