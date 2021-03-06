# Pybind11 Weaver

Pybind11 Weaver is a code generator to generate pybind11 code from c++ header files.
It will relieve you from creating a python binding. User could write all important (or necessary) pybind11 code,
and let Pybind11 Weaver do the rest tedious work.

It could turn the [sample.h](sample/enum/sample.h) into [sample.cc.inc](sample/enum/sample.cc.inc)
with [cfg.yaml](sample/enum/cfg.yaml). And after binding with a single line `auto update_guard = DeclFn(m);`
in [binding.cc](sample/enum/binding.cc),
all enum types in the header file will be available in the python module.

After building , the code below will give you the enum item of C++ `earth::creatures::Animal::DOG`.

```python
import enum_module

enum_module.earth.creatures.Animal.DOG
```

This project features that:

1. Highly customizable, while the default configuration is super simple, and is good for most cases.
2. Pure python package, just a `pip install` will make it ready to work.
3. Support mixing generated code with hand-writing code, and it is recommended to do.
4. Able to keep the module structure of the original c++ code.

## Background && Advice

This project was born from another internal project. The internal project's target was to make a python binding for a
**HUGE** developing c++ library.

Create python binding for a huge developing library brings some challenges:

1. The C++ library interface has tons of classes, functions, enums, create binding for all of them are **tedious**, and
   also
   **error-prone**.
2. The C++ library is developing, many new features are added every day, and existing code get frequent updates. It is
   really hard to track these updates by hand, so it is **hard to maintain** the python binding.
3. The C++ library does have some tricky parts that do not fit into the python convention, e.g., For some history
   reason, the library has some structs that are implemented in C style, something like `Foo::Create` and `Foo::Destroy`
   exist.
   However, python-binding do not need to carry such a burden, we could just re-design the struct to fit the python
   convention.
   So, **some part of the binding must be written by hand**.
4. **Huge** itself brings complexity. It is hard to make a generator smart enough to generate everything, so there must
   be a time that generator does not fit your needs, we should be able to write binding code by hand then.

For these reasons, We decided to create a tool. So we can write the binding code by hand, as much as we want, and let
the tool generate the rest. If this kind of usage follows your need, this project will be very happy to help you.

Recommended Usage:

1. Write binding code as you need. If you don't know what you need, just write nothing, you can do it later when you
   found some. Create a `cfg.yaml` file to describe what needs to be generated.
2. Use the generator to generate `inc` files.
3. Create a `binding.cc`, include all the `inc` file, call all the binding code.
4. Compile all code into a binary, and you are done.
5. Optionally, you can use the [pybind11-stubgen](https://github.com/sizmailov/pybind11-stubgen) to generate
   `.pyi` stub files, it will make your binding more readable to both human and MYPY in a static way.

## Installation

### PYPI

`python3 -m pip install pybind11-weaver`

### From Source

* Install from source

```bash
git clone https://github.com/edimetia3d/pybind11_weaver
python3 -m pip install $(pwd)/pybind11_weaver/
```

* Run from source (Editable/Develop Mode)

```bash
git clone https://github.com/edimetia3d/pybind11_weaver
python3 -m pip install $(pwd)/pybind11_weaver/ && python3 -m pip uninstall pybind11_weaver # A hacky way to install dependencies
export PYTHONPATH=$PYTHONPATH:$(pwd)/pybind11_weaver
alias pybind11_weaver=$(pwd)/pybind11_weaver/pybind11_weaver/main.py
```