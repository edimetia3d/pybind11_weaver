
# a python interpreter installed with pybind11-weaver is required
if (NOT DEFINED Python3_EXECUTABLE)
  return()
endif ()

add_test(NAME gen_unit_test
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/gen_unit_test.py
)
set_tests_properties(gen_unit_test PROPERTIES ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_LIST_DIR}/..")

add_test(NAME update_sample_test
    COMMAND ${Python3_EXECUTABLE} -m pybind11_weaver.main --config ${PROJECT_SOURCE_DIR}/sample/all_feature/cfg.yaml
)
set_tests_properties(update_sample_test PROPERTIES ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_LIST_DIR}/..")

add_subdirectory(sample_test)