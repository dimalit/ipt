#ifndef GEOMETRYOPENSPHERES_H
#define GEOMETRYOPENSPHERES_H

#include "tracer_interfaces.h"

struct GeometryOpenSpheres: public Geometry {
    virtual std::optional<surface_intersection> traceRay(glm::vec3 origin, glm::vec3 direction) const override;
};

#endif // GEOMETRYOPENSPHERES_H
