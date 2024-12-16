add_executable(glare-paint main.cpp)
target_include_directories(glare-paint PRIVATE ../include)

FIND_PACKAGE(X11 REQUIRED)
target_link_directories(glare-paint PRIVATE ${X11_LIB_DIRS})
target_include_directories(glare-paint PRIVATE ${X11_INCLUDE_DIR})
target_link_libraries(glare-paint PRIVATE ${X11_LIBRARIES} )

target_precompile_headers(glare-paint PRIVATE pch.h)
