cmake_minimum_required(VERSION 3.4...3.18)
project(pybind11_weaver_test CXX)

# this cmake is used only for testing the pybind11_weaver's header file
include(CTest)
enable_testing()
add_subdirectory(test)