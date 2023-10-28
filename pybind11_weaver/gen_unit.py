__all__ = [
    "GenUnit",
    "load_all_gu"
]

import datetime
import functools
from typing import List, Tuple

from pylibclang import cindex

from pybind11_weaver import config
from pybind11_weaver.utils import common


class GenUnit:
    io_config: config.IOConfig
    tu: cindex.TranslationUnit
    unsaved_file = Tuple[str, str]

    def __init__(self, io_config: config.IOConfig):
        self.io_config = io_config
        self._load_tu()
        self.creation_time: str = datetime.datetime.now().strftime("%m/%d/%Y, %H:%M:%S")

    def _load_tu(self, extra_content: str = ""):
        content = "\n".join(self.include_directives()) + extra_content
        unsaved_file = ("tmp.cpp", content)
        index = cindex.Index.create()
        tu = index.parse(unsaved_file[0],
                         unsaved_files=[unsaved_file],
                         args=["-x", "c++", "-fparse-all-comments", ] + self.io_config._cxx_flags)
        load_fail = False
        for diag in tu.diagnostics:
            print(diag.severity)
            print(diag.location)
            print(diag.spelling)
            print(diag.option)
            load_fail = True
        if load_fail:
            raise RuntimeError("Failed to parse the input file.")
        self.tu = tu
        self.unsaved_file = unsaved_file

    @functools.cache
    def include_files(self):
        files = []
        for f in self.io_config.inputs:
            files.append(f[1:-1])

        return files

    def reload_tu(self, new_content: str):
        unsaved_file = (self.unsaved_file[0], self.unsaved_file[1] + "\n" + new_content)
        self.tu.reparse([unsaved_file])
        self.unsaved_file = unsaved_file

    def include_directives(self) -> List[str]:
        return ["#include " + path for path in self.io_config.inputs]

    def is_cursor_in_inputs(self, cursor: cindex.Cursor):
        valid_files = self.include_files() + [self.unsaved_file[0]]
        file = cursor.location.file
        if file is None:
            return False
        cursor_filename = file.name
        in_src = False
        for tail in valid_files:
            if cursor_filename.endswith(tail):
                in_src = True
                break
        return (in_src
                and cursor.linkage != cindex.LinkageKind.CXLinkage_Internal
                and common.is_public(cursor)
                and common.is_visible(cursor,
                                      self.io_config.strict_visibility_mode))


def load_all_gu(file_or_content: str) -> List[GenUnit]:
    main_cfg = config.MainConfig.load(file_or_content)
    ret = []
    for io_cfg in main_cfg.io_configs:
        ret.append(GenUnit(io_cfg))
    return ret
