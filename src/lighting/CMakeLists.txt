add_library(lighting STATIC
    lighting.h
    lighting.cpp
)
target_link_libraries(lighting PUBLIC sampling)
target_include_directories(lighting PUBLIC ..)

# TODO Move check_ddfs to kind of separate library
add_executable(test_lighting test_lighting.cpp ../sampling/check_ddf.cpp)
target_link_libraries(test_lighting PRIVATE lighting)
enable_testing()
add_test(test_lighting test_lighting)


