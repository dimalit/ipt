add_library(lighting STATIC lighting.cpp)
target_include_directories(lighting PUBLIC ..)      # PUBLIC allows to apply this rule to referring targets (src in this case)

add_executable(test_lighting test_lighting.cpp)
target_link_libraries(test_lighting PRIVATE lighting)
# TODO include sampling in lighting or here?
target_link_libraries(test_lighting PRIVATE sampling)
enable_testing()
add_test(test_lighting test_lighting)


