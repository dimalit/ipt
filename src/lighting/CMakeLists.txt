add_library(lighting STATIC
    lighting.h
    lighting.cpp
)
target_link_libraries(lighting PUBLIC ddf)
target_include_directories(lighting PUBLIC ..)

add_executable(test_lighting test_lighting.cpp)
target_link_libraries(test_lighting PRIVATE lighting)
target_link_libraries(test_lighting PRIVATE ddf_testlib)
enable_testing()
add_test(test_lighting test_lighting)


