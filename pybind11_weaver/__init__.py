__VERSION__ = "0.1.2"

import os.path


def get_include():
    return os.path.join(os.path.abspath(os.path.dirname(__file__)), "include")
