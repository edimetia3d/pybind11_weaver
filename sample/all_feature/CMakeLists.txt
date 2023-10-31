add_subdirectory(c_lib)

include_directories(${CMAKE_CURRENT_LIST_DIR})
pybind11_add_module(all_feature binding.cc)
target_link_libraries(all_feature PRIVATE all_feature_c_lib)


