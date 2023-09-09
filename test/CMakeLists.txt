
# a python interpreter installed with pybind11-weaver is required
if (NOT DEFINED Python3_EXECUTABLE)
  return()
endif ()

add_test(NAME gen_unit_test
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/gen_unit_test.py
)
set_tests_properties(gen_unit_test
    PROPERTIES ENVIRONMENT "PYTHONPATH=${PROJECT_SOURCE_DIR}:$ENV{PYTHONPATH}")

add_subdirectory(sample_test)