import unittest

from pybind11_weaver import gen_unit


class TuLoaderTest(unittest.TestCase):

    def test_gen_unit_load(self):
        gen_units = gen_unit.load_all_gu("""
io_configs:
    - inputs: [<iostream>]
      output: "/path/to/output"
    - inputs: [<cstdio>]
      output: "/path/to/output2"
"""
                                         )
        self.assertEqual(len(gen_units), 2)
        for gu in gen_units:
            self.assertIsNotNone(gu.tu)


if __name__ == "__main__":
    unittest.main()
