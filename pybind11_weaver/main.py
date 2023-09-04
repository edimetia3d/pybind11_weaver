import argparse

import pybind11_weaver
from pybind11_weaver import gen_code


def parse_args():
    parser = argparse.ArgumentParser(description="Pybind11 command line interface.")
    parser.add_argument("--get_include",
                        action="store_true",
                        default=False,
                        help="Print the pybind11_weaver include path and exit.")
    parser.add_argument("--config",
                        type=str,
                        default=None,
                        help="Path to the config file")
    args, _ = parser.parse_known_args()
    if hasattr(args, "get_include") and args.get_include:
        print(pybind11_weaver.get_include())
        exit(0)
    return args


ARGS = parse_args()


def main():
    gen_code.gen_code(ARGS.config)


if __name__ == "__main__":
    main()
