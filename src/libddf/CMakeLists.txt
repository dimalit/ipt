add_library(ddf STATIC
    ddf.h
    ddf_detail.h
    ddf.cpp
)
target_include_directories(ddf INTERFACE .)

add_executable(test_ddf test_ddf.cpp check_ddf.h check_ddf.cpp)
target_link_libraries(test_ddf PRIVATE ddf)
enable_testing()
add_test(test_ddf test_ddf)
