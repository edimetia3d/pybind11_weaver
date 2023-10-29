# Pybind11 Weaver: Python Binding Code Generator

**Pybind11 Weaver** is a powerful code generator designed to automate the generation of pybind11 code from C++ header
files. It streamlines the process of creating Python bindings, enabling users to focus on writing critical pybind11 code
and offloading the tedious work to Pybind11 Weaver.

This tool takes a [sample.h](https://github.com/edimetia3d/pybind11_weaver/blob/main/sample/all_feature/sample.h) file
and transforms it into
a [sample_binding.cc.inc](https://github.com/edimetia3d/pybind11_weaver/blob/main/sample/all_feature/sample_binding.cc.inc)
file using [cfg.yaml](https://github.com/edimetia3d/pybind11_weaver/blob/main/sample/all_feature/cfg.yaml) as a guide.
Following the binding with a single line `auto update_guard = DeclFn(m);`
in [binding.cc](https://github.com/edimetia3d/pybind11_weaver/blob/main/sample/all_feature/binding.cc), all elements
from the header file become accessible in Python as demonstrated in
this [example](https://github.com/edimetia3d/pybind11_weaver/blob/main/test/sample_test/launch_module.py).

A more pragmatic example is available in [pylibclang](https://github.com/edimetia3d/pylibclang), a comprehensive Python
wrapper for libclang that uses Pybind11 Weaver to generate the binding code.

1. Its practicality stems from the fact that **Pybind11 Weaver operates on it** as well. Indeed, Pybind11 Weaver is
   self-hosted and generates the binding code for its own use.
2. Approximately 30k lines of C++ code are generated from a mere 10 lines
   of [cfg.yaml](https://github.com/edimetia3d/pylibclang/blob/master/c_src/cfg.yaml).
3. Some [binding code](https://github.com/edimetia3d/pylibclang/blob/master/c_src/binding.cc) is manually crafted to
   handle special cases and integrates seamlessly with the generated code.

[pylibtooling](https://github.com/edimetia3d/pylibtooling) is a much more advanced example that uses Pybind11 Weaver to
generate the binding code for [libtooling](https://clang.llvm.org/docs/LibTooling.html), and will be used to demonstrate
the capabilities of Pybind11 Weaver when working with large C++ only libraries.

## Key Features

1. **Highly Customizable:** While the default configuration is super simple and suitable for most cases, it allows for
   high customization.
2. **Ease of Use:** As a pure Python package, a simple `pip install` gets it ready to work.
3. **Versatility:** All generated code is under your control, you can easily modify/enhance/disable any part of
   generated
   code, and all generated code will work with your hand-written code seamlessly.
4. **Structure Preservation:** It retains the module structure of the original C++ code.

## Features & Roadmap

- [x] Binding for Enum
- [x] Binding for Namespace (as submodule)
- [x] Binding for Function, with support of function overloading
- [x] Binding for C style function pointer (usually used as callback functions)
- [x] Binding for opaque pointer and pointer to incomplete type
- [ ] Binding for Operator overloading
- [x] Binding for Class method, method overloading, static method, static method overloading, constructor, constructor
  overloading, class field
- [x] Trampoline class for virtual function
- [x] Binding for concreate template instance, that includes: implicit(explicit) class(struct) template instantiation,
  full class(struct) template specialization, extern function template instance declaration.
- [x] Support class inheritance hierarchy
- [x] Auto ignore symbols by : Linkage (e.g. `static`), visibility (e.g. `visibility=hidden`), member access
  control (e.g. `private`, `protected`)
- [x] Docstring generation from c++ doxygen style comment
- [x] Namespace hierarchy to Python module hierarchy
- [x] Dynamic update/disable binding by API call.
- [x] Static update/disable binding by define macro (Mainly used to disable wrong binding code to avoid compilation
  error)
- [ ] Auto snake case

## Background & Recommendations

This project originated from an internal project aimed at creating a Python binding for a **LARGE** developing C++
library. This posed significant challenges:

1. The C++ library interface contained a vast number of classes, functions, and enums. Creating bindings for all these
   elements was not only **tedious** but also **error-prone**.
2. Because the C++ library was under active development, staying updated with daily additions and frequent code
   modifications was a **maintenance challenge**.
3. Some aspects of the C++ library, due to historical reasons, were incompatible with Python conventions, necessitating
   **hand-written binding codes**.
4. The sheer size of the library added to the complexity, making it difficult to develop a generator smart enough to
   handle everything, hence the need for manual binding code writing.

In light of these challenges, I designed Pybind11 Weaver as a tool to generate the majority of the binding code,
leaving users to handcraft the remaining parts as needed. If this approach suits your needs, this tool will be a
valuable asset.

**Typical workflow:**

Though most features should work out of the box, the more your API looks like "C With Class", the higher chance Pybind11
Weaver will do all the work for you. If you use too many advanced C++ features, you may need to write some binding
code by yourself.

1. Create a `cfg.yaml` file, mainly to tell the generator which files to parse.
2. Use Pybind11 Weaver to generate files, like `pybind11-weaver --config cfg.yaml`.
3. Create a `binding.cc`, include the generated files, and call the binding code.
4. Disable some generated binding code by define some macro, if there is any compilation error.
5. Add some custom code to replace part of the generated code, or adding some new binding that generator had not
   exported.
6. Compile all code into a pybind11 module.
7. Optionally, use [pybind11-stubgen](https://github.com/sizmailov/pybind11-stubgen) to generate `.pyi` stub files,
   enhancing readability for both humans and MYPY in a static way.
8. Test the module in Python, find bugs, and go to step 5 to fix them.

Also, if you encountered too many problems, you are welcome to open an issue at github, or create a PR to fix it.

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

The Pybind11 Weaver operates under the hood by utilizing [libclang](https://clang.llvm.org/), a library that parses C++
header files. This enables us to obtain all APIs from the header file, which are then used to generate the binding code
on your behalf.

Notably, only header files are required, as we need declarations, not definitions. However, to ensure accurate parsing
of the code, some compiler flags, especially for macros, are necessary.

The code generated is structured into a `struct`:

1. During the construction of the struct, it creates some Pybind11 objects, such as `pybind11::class_`
   or `pybind11::enum_`.
2. When the `Update()` API is invoked, the Pybind11 object experiences an update.

The use of a struct permits us to:

* Separate the processes of object creation and updates, ensuring that Pybind11 consistently acknowledges all exported
  classes, which aids in the generation of accurate documentation.
* Increase the readability of the generated code, making it simpler to debug.
* Simplify customization, as you can easily inherit the struct and override or reimplement necessary elements.
