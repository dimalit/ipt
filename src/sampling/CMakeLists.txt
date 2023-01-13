add_library(sampling STATIC
    sampling.h
    sampling.cpp
)

add_executable(test_sampling test_sampling.cpp check_ddf.h check_ddf.cpp)
target_link_libraries(test_sampling PRIVATE sampling)
enable_testing()
add_test(test_sampling test_sampling)
