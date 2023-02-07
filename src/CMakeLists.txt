add_subdirectory(libddf)
add_subdirectory(lighting)
add_subdirectory(geometry)
add_subdirectory(libcatch2)

file( GLOB sources "*.cpp" "*.h")
add_library(ipt_lib STATIC ${sources})
# PUBLIC allows to apply include paths from these libs to main:
target_link_libraries(ipt_lib PUBLIC ddf)
target_link_libraries(ipt_lib PUBLIC lighting)
target_link_libraries(ipt_lib PUBLIC geometry)

add_executable(ipt main.cpp)
target_link_libraries(ipt PRIVATE ipt_lib)
