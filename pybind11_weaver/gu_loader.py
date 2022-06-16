__all__ = [
    "GenUnit",
    "load_gen_unit_from_config"
]

import collections
import copy
import datetime
import os.path
import sysconfig
from typing import List

import yaml
from clang import cindex

import pybind11_weaver.third_party.ccsyspath as ccsyspath


def get_default_include_flags(compiler_path: str = None) -> List[str]:
    try_path = ["c++", "g++", "clang++"]
    if compiler_path is not None:
        try_path = [compiler_path] + try_path
    cxx_sys_path = []
    for compiler in try_path:
        cxx_sys_path = ccsyspath.system_include_paths(compiler)
        if len(cxx_sys_path) > 0:
            break
    assert len(cxx_sys_path) > 0, "Can not find c++ headers"
    full_list = [d.decode("utf-8") for d in cxx_sys_path] + [
        sysconfig.get_path("include"),
    ]
    return ["-I" + path for path in full_list]


def cleanup_config(cfg):
    """Fill optional values to its default."""

    def safe_update(target, default):
        for key in default:
            if key not in target:
                target[key] = copy.deepcopy(default[key])

    if cfg is None:
        cfg = {}
    safe_update(cfg, {
        "common_config": {},
        "io_configs": []
    })
    safe_update(cfg["common_config"], {
        "compiler": None,
        "cxx_flags": [],
        "include_directories": [],
        "namespace": ""
    })
    for entry in cfg["io_configs"]:
        safe_update(entry, {
            "inputs": [],
            "output": "",
            "namespace": "",
            "extra_cxx_flags": [],
        })
    return cfg


def load_config(file_or_content: str):
    if os.path.exists(file_or_content):  # it is a file
        with open(file_or_content, "r") as yml:
            cfg = yaml.safe_load(yml)
    else:  # it is a string
        cfg = yaml.safe_load(file_or_content)
    return cleanup_config(cfg)


def load_tu(file_list: List[str], cxx_flags: List[str]):
    def clean_file_list():
        files_cleand = []
        for f in file_list:
            if f.startswith('"') or f.startswith("<"):
                files_cleand.append(f)
            else:
                files_cleand.append(f'"{f}"')
        return files_cleand

    content = "\n".join(["#include " + path for path in clean_file_list()])
    index = cindex.Index.create()
    tu = index.parse("tmp.cpp",
                     unsaved_files=[("tmp.cpp", content)],
                     args=["-x", "c++", "-fparse-all-comments", ] + cxx_flags)
    load_fail = False
    for diag in tu.diagnostics:
        print(diag.severity)
        print(diag.location)
        print(diag.spelling)
        print(diag.option)
        load_fail = True
    if load_fail:
        return None
    return tu, content


class GenUnit:
    Options = collections.namedtuple("Options", ("output", "namespace"))

    def __init__(self, tu, src_files: List[str], options: Options):
        self.creation_time: str = datetime.datetime.now().strftime("%m/%d/%Y, %H:%M:%S")
        self.tu = tu
        self.src_files: List[str] = src_files
        self.options: GenUnit.Options = options


def load_gen_unit_from_config(file_or_content: str) -> List[GenUnit]:
    cfg = load_config(file_or_content)
    default_headers = get_default_include_flags(cfg["common_config"]["compiler"])
    common_flags = cfg["common_config"]["cxx_flags"] + default_headers
    common_flags = common_flags + ["-I" + path for path in cfg["common_config"]["include_directories"]]
    ret = []
    for entry in cfg["io_configs"]:
        src_tu, _ = load_tu(entry["inputs"], common_flags + entry["extra_cxx_flags"])
        assert src_tu is not None
        ret.append(
            GenUnit(src_tu, entry["inputs"], GenUnit.Options(output=entry["output"], namespace=entry["namespace"])))
    return ret