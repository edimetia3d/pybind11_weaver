__all__ = [
    "GenUnit",
    "load_gen_unit_from_config"
]

import copy
import datetime
import os.path
import sysconfig
from typing import List, Tuple, Optional, Dict, Any
from glob import glob

import pybind11
import yaml
from pylibclang import cindex

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
            "gen_docstring": True,
            "strict_visibility_mode": False,
        })
        entry["inputs"] = _clean_paths(entry["inputs"])
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


def _clean_paths(file_list: List[str]) -> List[str]:
    files_cleand = []
    for f in file_list:
        if f.startswith('"') or f.startswith("<"):
            files_cleand.append(f)
        elif "glob" in f:
            glob_list = eval(f)
            files_cleand.extend(_clean_paths(glob_list))
        else:
            files_cleand.append(f'"{f}"')
    return files_cleand


def _file_paths_to_include(file_list: List[str]) -> List[str]:
    return ["#include " + path for path in file_list]


def load_tu(file_list: List[str], cxx_flags: List[str], extra_content: str = "") -> Tuple[
    Optional[cindex.TranslationUnit], Optional[List[Tuple[str, str]]]]:
    content = "\n".join(_file_paths_to_include(file_list)) + extra_content
    index = cindex.Index.create()
    unsaved_files = [("tmp.cpp", content)]
    tu = index.parse("tmp.cpp",
                     unsaved_files=unsaved_files,
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
    return tu, unsaved_files


class GenUnit:
    class IOConifg:
        def __init__(self, key_values: Dict[str, Any]):
            for k, v in key_values.items():
                setattr(self, k, v)

    def __init__(self, tu, io_config: Dict[str, Any], cxx_flags: List[str], unsaved_files: List[Tuple[str, str]]):
        self.creation_time: str = datetime.datetime.now().strftime("%m/%d/%Y, %H:%M:%S")
        self.tu: cindex.TranslationUnit = tu
        self.io_config = GenUnit.IOConifg(io_config)
        self.cxx_flags = cxx_flags
        self.unsaved_files = unsaved_files
        self.src_files: List[str] = self._clean_src_file_to_relative(cxx_flags, io_config["inputs"])

    def _clean_src_file_to_relative(self, cxx_flags: List[str], file_list: List[str]) -> List[str]:

        def _get_abs_includes_prefix():
            ret = []
            for flag in cxx_flags:
                if flag.startswith("-I") and os.path.isabs(flag[2:]):
                    ret.append(os.path.abspath(flag[2:]))
            return ret

        abs_prefix = _get_abs_includes_prefix()

        def _try_remove_abs_prefix(path: str) -> str:
            path = os.path.abspath(path)
            for prefix in abs_prefix:
                if path.startswith(prefix):
                    return os.path.relpath(path, prefix)
            return path

        ret = []
        for i in range(len(file_list)):
            f = file_list[i]
            if f.startswith('"') and os.path.isabs(f[1:-1]):
                ret.append(f'"{_try_remove_abs_prefix(f[1:-1])}"')
            else:
                ret.append(f)
        return ret

    def include_files(self):
        files = []
        for f in self.src_files:
            files.append(f[1:-1])

        return files

    def src_file_includes(self) -> List[str]:
        return _file_paths_to_include(self.src_files)


def load_gen_unit_from_config(file_or_content: str) -> List[GenUnit]:
    cfg = load_config(file_or_content)
    default_headers = get_default_include_flags(cfg["common_config"]["compiler"])
    common_flags = cfg["common_config"]["cxx_flags"] + default_headers
    common_flags = common_flags + ["-I" + path for path in cfg["common_config"]["include_directories"]]
    ret = []
    for io_cfg in cfg["io_configs"]:
        cxx_flags = common_flags + io_cfg["extra_cxx_flags"]
        tu, unsaved_files = load_tu(io_cfg["inputs"], cxx_flags)
        assert tu is not None
        ret.append(GenUnit(tu, io_cfg, cxx_flags, unsaved_files))
    return ret
