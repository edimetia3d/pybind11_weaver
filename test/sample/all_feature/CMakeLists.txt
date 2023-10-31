# may use`python3 -m pybind11 --cmakedir`

add_test(NAME generate_all_feature_binding
    COMMAND ${Python3_EXECUTABLE} -m pybind11_weaver.main --config ${PROJECT_SOURCE_DIR}/sample/all_feature/cfg.yaml
)

add_test(build_all_feature_extension
    "${CMAKE_COMMAND}"
    --build "${CMAKE_BINARY_DIR}"
    --config "$<CONFIG>"
    --target all_feature
)
set_tests_properties(build_all_feature_extension PROPERTIES DEPENDS "generate_all_feature_binding")


add_test(NAME test_all_feature
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/test_all_feature.py
)
set_tests_properties(test_all_feature PROPERTIES DEPENDS "build_all_feature_extension")
set_tests_properties(test_all_feature PROPERTIES ENVIRONMENT "PYTHONPATH=${PROJECT_BINARY_DIR}")

