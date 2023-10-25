# may use`python3 -m pybind11 --cmakedir`
if (NOT DEFINED pybind11_DIR)
  return()
endif ()

find_package(pybind11 REQUIRED CONFIG)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../../pybind11_weaver/include)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../..)
pybind11_add_module(all_feature_module ${PROJECT_SOURCE_DIR}/sample/all_feature/binding.cc)
add_library(all_feature_lib ${PROJECT_SOURCE_DIR}/sample/all_feature/sample.cc)
target_link_libraries(all_feature_module PRIVATE all_feature_lib)

add_test(build_extension
    "${CMAKE_COMMAND}"
    --build "${CMAKE_BINARY_DIR}"
    --config "$<CONFIG>"
    --target all_feature_module
)

add_test(NAME launch_test_module
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/launch_module.py
)
set_tests_properties(launch_test_module PROPERTIES DEPENDS "build_extension;update_sample_test")

set_tests_properties(launch_test_module
    PROPERTIES ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}:$ENV{PYTHONPATH}")

