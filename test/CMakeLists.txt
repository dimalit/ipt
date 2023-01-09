file( GLOB test_sources "*.cpp" "*.h")

add_executable(test_ipt ${test_sources})
target_include_directories(test_ipt PRIVATE ../src)
target_link_libraries(test_ipt PRIVATE ipt_lib)

enable_testing()
add_test(test_ipt test_ipt)
