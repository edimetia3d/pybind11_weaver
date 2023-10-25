from typing import List, Optional
import glob
import os

import yaml
import attrs
import sysconfig
import pybind11
import pybind11_weaver.third_party.ccsyspath as ccsyspath


@attrs.define
class IOConfig:
    inputs: List[str] = attrs.field(factory=list)
    output: str = ""
    decl_fn_name: str = "DeclFn"
    root_module_namespace: str = ""
    _cxx_flags: List[str] = None
    extra_cxx_flags: List[str] = attrs.field(factory=list)
    gen_docstring: bool = True
    strict_visibility_mode: bool = False

    def normalize(self, common_config: "CommonConfig"):
        self._cxx_flags = common_config.cxx_flags + self.extra_cxx_flags
        self._normalize_inputs()

    def _normalize_inputs(self):
        self.inputs = self._to_valid_include_path(self.inputs)
        self._inputs_to_relative_path()

    @staticmethod
    def _to_valid_include_path(file_list: List[str]) -> List[str]:
        files_cleand = []
        for f in file_list:
            if f.startswith('"') or f.startswith("<"):
                files_cleand.append(f)
            elif "glob" in f:
                glob_list = eval(f, {"glob": glob.glob})
                files_cleand.extend(IOConfig._to_valid_include_path(glob_list))
            else:
                files_cleand.append(f'"{f}"')
        return files_cleand

    def _inputs_to_relative_path(self):

        def _get_abs_includes_prefix():
            ret = []
            for flag in self._cxx_flags:
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

        for i in range(len(self.inputs)):
            f = self.inputs[i]
            if f.startswith('"') and os.path.isabs(f[1:-1]):
                f = f'"{_try_remove_abs_prefix(f[1:-1])}"'
                self.inputs[i] = f


@attrs.define
class CommonConfig:
    compiler: Optional[str] = None
    cxx_flags: List[str] = attrs.field(factory=list)
    include_directories: List[str] = attrs.field(factory=list)

    def normalize(self):
        include_sys_flags = self._get_default_include_flags()
        include_usr_flags = ["-I" + path for path in self.include_directories]
        self.cxx_flags = self.cxx_flags + include_sys_flags + include_usr_flags

    def _get_default_include_flags(self) -> List[str]:
        try_to_use = ["c++", "g++", "clang++"]
        if os.environ.get("CXX") is not None:
            try_to_use = [os.environ.get("CXX")] + try_to_use
        if self.compiler is not None:
            try_to_use = [self.compiler] + try_to_use
        cxx_sys_path = []
        for compiler in try_to_use:
            cxx_sys_path = ccsyspath.system_include_paths(compiler)
            if len(cxx_sys_path) > 0:
                self.compiler = compiler
                break
        assert len(cxx_sys_path) > 0, "Can not find c++ headers"
        full_list = [d.decode("utf-8") for d in cxx_sys_path] + [
            sysconfig.get_path("include"),
            pybind11.get_include(),
        ]
        return ["-I" + path for path in full_list]


def _safe_load_one_cls(cls, possible_values):
    instance = cls()
    attrs.resolve_types(cls)
    for field in attrs.fields(cls):
        if field.name in possible_values:
            if attrs.has(field.type):
                setattr(instance, field.name,
                        _safe_load_one_cls(field.type, possible_values[field.name]))
            else:
                setattr(instance, field.name, possible_values[field.name])
    return instance


@attrs.define
class MainConfig:
    common_config: CommonConfig = attrs.field(factory=CommonConfig)
    io_configs: List[IOConfig] = attrs.Factory(factory=list)

    @staticmethod
    def load(file_or_content: str) -> "MainConfig":
        content = file_or_content
        if os.path.exists(content):  # it is a file
            with open(content, "r") as yml:
                content = yml.read()
                content = content.replace("${CFG_DIR}", os.path.dirname(os.path.abspath(file_or_content)))
        cfg = yaml.safe_load(content)

        main_config = _safe_load_one_cls(MainConfig, cfg)
        main_config.common_config.normalize()
        for i, io_config in enumerate(main_config.io_configs):
            new_io_cfg = _safe_load_one_cls(IOConfig, io_config)
            new_io_cfg.normalize(main_config.common_config)
            main_config.io_configs[i] = new_io_cfg
        return main_config
