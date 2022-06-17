import argparse

import pybind11_weaver
from pybind11_weaver import entity
from pybind11_weaver import gen_unit


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
    gus = gen_unit.load_gen_unit_from_config(ARGS.config)
    for gu in gus:
        entity.get_all_entities(gu)


if __name__ == "__main__":
    main()
