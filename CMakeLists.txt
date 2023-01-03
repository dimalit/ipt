cmake_minimum_required(VERSION 3.10)

project(ipt)

set(CMAKE_CXX_STANDARD 17)

file( GLOB sources "src/*.cpp" "src/*.h")
add_executable(ipt ${sources})

target_include_directories(ipt PRIVATE include)

