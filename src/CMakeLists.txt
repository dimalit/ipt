add_subdirectory(libddf)
add_subdirectory(lighting)
add_subdirectory(geometry)
add_subdirectory(libcatch2)

FIND_PACKAGE(X11)
FIND_PACKAGE(Threads REQUIRED)

file( GLOB sources "*.cpp" "*.h")
add_library(ipt_lib STATIC ${sources})
# PUBLIC allows to apply include paths from these libs to main:
target_link_libraries(ipt_lib PUBLIC ddf)
target_link_libraries(ipt_lib PUBLIC lighting)
target_link_libraries(ipt_lib PUBLIC geometry)

add_executable(ipt main.cpp)
target_link_libraries(ipt PRIVATE ipt_lib)
target_link_libraries(ipt PRIVATE gui)

add_library(gui gui.h gui.cpp)
target_link_libraries(gui PRIVATE ipt_lib)
target_link_libraries(gui PRIVATE ${CMAKE_THREAD_LIBS_INIT})
target_precompile_headers(gui PRIVATE pch.h)
if(X11_FOUND)
  target_link_directories(gui PRIVATE ${X11_LIB_DIRS})
  target_include_directories(gui PRIVATE ${X11_INCLUDE_DIR})
  target_link_libraries(gui PRIVATE ${X11_LIBRARIES} )
endif()
