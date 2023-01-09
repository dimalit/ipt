add_subdirectory(sampling)

file( GLOB sources "*.cpp" "*.h")
add_library(ipt_lib STATIC ${sources})
target_link_libraries(ipt_lib PRIVATE sampling)

add_executable(ipt main.cpp)
target_link_libraries(ipt PRIVATE ipt_lib)
