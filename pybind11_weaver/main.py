import argparse

from pybind11_weaver import entity
from pybind11_weaver import gen_unit


def parse_args():
    parser = argparse.ArgumentParser(description="Pybind11 command line interface.")
    parser.add_argument("--config",
                        type=str,
                        required=True,
                        help="Path to the config file")
    return parser.parse_args()


ARGS = parse_args()


def main():
    gus = gen_unit.load_gen_unit_from_config(ARGS.config)
    for gu in gus:
        entity.get_all_entities(gu)


if __name__ == "__main__":
    main()
