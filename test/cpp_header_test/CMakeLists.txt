if (NOT DEFINED pybind11_DIR)
    return()
endif ()

find_package(pybind11 REQUIRED CONFIG)
find_package(Python3 COMPONENTS Interpreter Development)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../../pybind11_weaver/include)

pybind11_add_module(test_module module.cpp)

add_test(NAME launch_test_module
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/launch_module.py
        DEPENDS test_module
        )
set_tests_properties(launch_test_module
        PROPERTIES ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}:$ENV{PYTHONPATH}")

