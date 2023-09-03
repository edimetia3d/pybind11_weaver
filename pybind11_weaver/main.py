import argparse

import pybind11_weaver
from pybind11_weaver import gen_code


def _handle_optional_args(args):
    if hasattr(args, "get_include") and args.get_include:
        print(pybind11_weaver.get_include())
        exit(0)


def _add_required_args(parser: argparse.ArgumentParser):
    parser.add_argument("--config",
                        type=str,
                        required=True,
                        help="Path to the config file")


def _add_optional_args(parser: argparse.ArgumentParser):
    parser.add_argument("--get_include",
                        action="store_true",
                        default=False,
                        help="Print the pybind11_weaver include path and exit.")


def parse_args():
    parser = argparse.ArgumentParser(description="Pybind11 command line interface.")
    _add_optional_args(parser)
    _handle_optional_args(parser.parse_known_args())
    _add_required_args(parser)
    return parser.parse_args()


ARGS = parse_args()


def main():
    gen_code.gen_code(ARGS.config)


if __name__ == "__main__":
    main()
