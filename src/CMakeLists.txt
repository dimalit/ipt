add_subdirectory(sampling)
add_subdirectory(lighting)

file( GLOB sources "*.cpp" "*.h")
add_library(ipt_lib STATIC ${sources})
# PUBLIC allows to apply include paths from these libs to main:
target_link_libraries(ipt_lib PUBLIC sampling)
target_link_libraries(ipt_lib PUBLIC lighting)

add_executable(ipt main.cpp)
target_link_libraries(ipt PRIVATE ipt_lib)
