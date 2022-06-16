import argparse

from pybind11_weaver import gu_loader


def parse_args():
    parser = argparse.ArgumentParser(description="Pybind11 command line interface.")
    parser.add_argument("--config",
                        type=str,
                        required=True,
                        help="Path to the config file")
    return parser.parse_args()


ARGS = parse_args()


def main():
    gu_loader.load_gen_unit_from_config(ARGS.config)


if __name__ == "__main__":
    main()
