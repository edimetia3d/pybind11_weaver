# Pybind11 Weaver: Python Binding Code Generator

**Pybind11 Weaver** is a powerful code generator designed to automate the generation of pybind11 code from C++ header files. It streamlines the process of creating Python bindings, enabling users to focus on writing critical pybind11 code and offloading the tedious work to Pybind11 Weaver.

This tool takes a [sample.h](sample/all_feature/sample.h) file and transforms it into a [sample_binding.cc.inc](sample/all_feature/sample_binding.cc.inc) file using [cfg.yaml](sample/all_feature/cfg.yaml) as a guide. Following the binding with a single line `auto update_guard = DeclFn(m);` in [binding.cc](sample/all_feature/binding.cc), all elements from the header file become accessible in Python as demonstrated in this [example](test/sample_test/launch_module.py).


## Key Features

1. **Highly Customizable:** While the default configuration is super simple and suitable for most cases, it allows for high customization.
2. **Ease of Use:** As a pure Python package, a simple `pip install` gets it ready to work.
3. **Versatility:** It supports the merging of generated code with hand-written code, a practice we highly recommend.
4. **Structure Preservation:** It retains the module structure of the original C++ code.

## Features & Roadmap
- [x] Namespace hierarchy to Python submodules
- [x] Enum
- [ ] Enum doc
- [x] Function, function overload
- [ ] Function doc
- [x] Class method, method overloading, static method, static method overloading, constructor, constructor overloading
- [x] Class field
- [x] Class access control
- [ ] Trampoline class for virtual function
- [ ] Class doc, method doc, field doc
- [x] Support working with hand-written code
- [ ] Auto snake case

## Background & Recommendations

This project originated from an internal project aimed at creating a Python binding for a **LARGE** developing C++ library. This posed significant challenges:

1. The C++ library interface contained a vast number of classes, functions, and enums. Creating bindings for all these elements was not only **tedious** but also **error-prone**.
2. Because the C++ library was under active development, staying updated with daily additions and frequent code modifications was a **maintenance challenge**.
3. Some aspects of the C++ library, due to historical reasons, were incompatible with Python conventions, necessitating **hand-written binding codes**.
4. The sheer size of the library added to the complexity, making it difficult to develop a generator smart enough to handle everything, hence the need for manual binding code writing.

In light of these challenges, we designed Pybind11 Weaver as a tool to generate the majority of the binding code, leaving users to handcraft the remaining parts as needed. If this approach suits your needs, this tool will be a valuable asset.

**Recommended Usage:**

1. Create a `cfg.yaml` file to describe what needs to be generated.
2. Use the generator to create `inc` files.
3. Create a `binding.cc`, include all the `inc` files, and call all the binding code.
4. Compile all code into a binary to finish.
5. Optionally, use [pybind11-stubgen](https://github.com/sizmailov/pybind11-stubgen) to generate `.pyi` stub files, enhancing readability for both humans and MYPY in a static way.

## Installation

### Via PYPI

```bash
python3 -m pip install pybind11-weaver
```

### From Source

* To install from source:

```bash
git clone https://github.com/edimetia3d/pybind11_weaver
python3 -m pip install $(pwd)/pybind11_weaver/
```

* To run from source (Editable/Develop Mode):

```bash
git clone https://github.com/edimetia3d/pybind11_weaver
python3 -m pip install -e $(pwd)/pybind11_weaver/ -v --config-settings editable_mode=compat
```
## How it works

The Pybind11 Weaver operates under the hood by utilizing [libclang](https://clang.llvm.org/), a library that parses C++ header files. This enables us to obtain all APIs from the header file, which are then used to generate the binding code on your behalf.

Notably, only header files are required, as we need declarations, not definitions. However, to ensure accurate parsing of the code, some compiler flags, especially for macros, are necessary.

The code generated is structured into a `struct`:
1. During the construction of the struct, it creates some Pybind11 objects, such as `pybind11::class_` or `pybind11::enum_`.
2. When the `Update()` API is invoked, the Pybind11 object experiences an update.

The use of a struct permits us to:
* Separate the processes of object creation and updates, ensuring that Pybind11 consistently acknowledges all exported classes, which aids in the generation of accurate documentation.
* Increase the readability of the generated code, making it simpler to debug.
* Simplify customization, as you can easily inherit the struct and override or reimplement necessary elements.
