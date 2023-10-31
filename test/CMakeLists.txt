
add_test(NAME gen_unit_test
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/gen_unit_test.py
)

add_subdirectory(sample)