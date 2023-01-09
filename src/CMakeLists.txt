file( GLOB sources "*.cpp" "*.h")

add_library(ipt_lib STATIC ${sources})

add_executable(ipt main.cpp)
target_link_libraries(ipt PRIVATE ipt_lib)


