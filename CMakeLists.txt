cmake_minimum_required(VERSION 3.10)

project(ipt)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(ipt main.cpp)
target_include_directories(ipt PRIVATE src)
target_link_libraries(ipt PRIVATE ipt_lib)

add_subdirectory(src)

add_subdirectory(test)
