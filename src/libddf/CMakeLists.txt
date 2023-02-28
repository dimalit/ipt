add_library(ddf STATIC
    ddf.h
    ddf_detail.h
    ddf.cpp
)
target_include_directories(ddf INTERFACE .)

add_library(ddf_testlib STATIC
    check_ddf.h
    check_ddf.cpp
)
target_include_directories(ddf_testlib INTERFACE .)

add_executable(test_ddf test_ddf.cpp)
target_link_libraries(test_ddf PRIVATE ddf)
target_link_libraries(test_ddf PRIVATE ddf_testlib)

FIND_PACKAGE(X11)
FIND_PACKAGE(Threads REQUIRED)
target_link_libraries(test_ddf PRIVATE ${CMAKE_THREAD_LIBS_INIT})
if(X11_FOUND)
  target_link_directories(test_ddf PRIVATE ${X11_LIB_DIRS})
  target_include_directories(test_ddf PRIVATE ${X11_INCLUDE_DIR})
  target_link_libraries(test_ddf PRIVATE ${X11_LIBRARIES} )
endif()

enable_testing()
target_link_libraries(test_ddf PRIVATE catch2)
add_test(test_ddf test_ddf)
