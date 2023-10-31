# Cmake project is mainly to test the pybind11_weaver, not used to build the pip package
# use setup.py to build the pip package
cmake_minimum_required(VERSION 3.18)
project(pybind11_weaver CXX)

# this cmake is used only for testing the pybind11_weaver's header file
include(CTest)
enable_testing()

# try find pybind11 and enable sample build
find_package(pybind11 CONFIG)
if (pybind11_FOUND)
  add_subdirectory(sample)
else ()
  message(STATUS "pybind11 not found , sample extension build disabled")
endif ()

# try find Python3
find_package(Python3 COMPONENTS Interpreter Development)
# enable test when pybind11 and Python3 both found
if (pybind11_FOUND AND Python3_FOUND)
  # test if pybind11_weaver could be imported
  set(test_expr "import pybind11_weaver")
  execute_process(COMMAND ${Python3_EXECUTABLE} -c ${test_expr}
      RESULT_VARIABLE test_result
      OUTPUT_QUIET
      ERROR_QUIET
  )
  if (NOT ${test_result} EQUAL 0)
    message(WARNING "Test disabled for pybind11_weaver import failure, may be dependency not installed?")
  else ()
    add_subdirectory(test)
  endif ()
else ()
  message(STATUS "Python3 not found, all test disabled")
endif ()
