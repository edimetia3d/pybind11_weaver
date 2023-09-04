__all__ = [
    "GenUnit",
    "load_gen_unit_from_config"
]

import collections
import copy
import datetime
import os.path
import sysconfig
from typing import List, Tuple, Optional

import pybind11
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
        pybind11.get_include(),
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
    })
    for entry in cfg["io_configs"]:
        safe_update(entry, {
            "inputs": [],
            "output": "",
            "decl_fn_name": "DeclFn",
            "root_module_namespace": "",
            "extra_cxx_flags": [],
        })
    return cfg


def load_config(file_or_content: str):
    if os.path.exists(file_or_content):  # it is a file
        with open(file_or_content, "r") as yml:
            content = yml.read()
            content = content.replace("${CFG_DIR}", os.path.dirname(os.path.abspath(file_or_content)))
            cfg = yaml.safe_load(content)
    else:  # it is a string
        cfg = yaml.safe_load(file_or_content)
    return cleanup_config(cfg)


def _file_paths_to_include(file_list: List[str]) -> List[str]:
    files_cleand = []
    for f in file_list:
        if f.startswith('"') or f.startswith("<"):
            files_cleand.append(f)
        else:
            files_cleand.append(f'"{f}"')
    return ["#include " + path for path in files_cleand]


def load_tu(file_list: List[str], cxx_flags: List[str], extra_content: str = "") -> Tuple[
    Optional[cindex.TranslationUnit], str]:
    content = "\n".join(_file_paths_to_include(file_list)) + extra_content
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
        return None, None
    return tu, content


class GenUnit:
    Options = collections.namedtuple("Options", ("output", "decl_fn_name", "root_module_namespace"))

    def __init__(self, tu, src_files: List[str], options: Options, visibility_hidden: bool):
        self.creation_time: str = datetime.datetime.now().strftime("%m/%d/%Y, %H:%M:%S")
        self.tu: cindex.TranslationUnit = tu
        self.src_files: List[str] = src_files
        self.options: GenUnit.Options = options
        self.fvisibility_hidden = visibility_hidden

    def is_visible(self, cursor: cindex.Cursor):
        """Utility method to check if a cursor is visible in the current translation unit."""
        if cursor.kind in [cindex.CursorKind.ENUM_DECL, cindex.CursorKind.NAMESPACE]:
            return True
        visible = not self.fvisibility_hidden
        for c in cursor.get_children():
            if c.kind == cindex.CursorKind.VISIBILITY_ATTR:
                if c.spelling == "default":
                    visible = True
                elif c.spelling == "hidden":
                    visible = False
        return visible

    def src_file_tail_names(self):
        files = []
        for f in self.src_files:
            if f.startswith("<") or f.startswith('"'):
                files.append(f[1:-1])
            else:
                files.append(f)
        return files

    def src_file_includes(self) -> List[str]:
        return _file_paths_to_include(self.src_files)


def load_gen_unit_from_config(file_or_content: str) -> List[GenUnit]:
    cfg = load_config(file_or_content)
    default_headers = get_default_include_flags(cfg["common_config"]["compiler"])
    common_flags = cfg["common_config"]["cxx_flags"] + default_headers
    common_flags = common_flags + ["-I" + path for path in cfg["common_config"]["include_directories"]]
    ret = []
    for entry in cfg["io_configs"]:
        cxx_flags = common_flags + entry["extra_cxx_flags"]
        src_tu, _ = load_tu(entry["inputs"], cxx_flags)
        assert src_tu is not None
        ret.append(
            GenUnit(src_tu, entry["inputs"],
                    GenUnit.Options(output=entry["output"],
                                    decl_fn_name=entry["decl_fn_name"],
                                    root_module_namespace=entry["root_module_namespace"]),
                    visibility_hidden=("-fvisibility=hidden" in " ".join(cxx_flags))))
    return ret
