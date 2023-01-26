add_library(geometry STATIC
    geometric_utils.h
    geometric_utils.cpp
    GeometryFloor.h
    GeometryFloor.cpp
    GeometrySphereInBox.h
    GeometrySphereInBox.cpp
    GeometryOpenSpheres.h
    GeometryOpenSpheres.cpp
)
target_link_libraries(geometry PRIVATE ddf)
target_include_directories(geometry PRIVATE ..)
