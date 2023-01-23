#ifndef GEOMETRYFLOOR_H
#define GEOMETRYFLOOR_H

#include <tracer_interfaces.h>

struct GeometryFloor: public Geometry {
    virtual std::optional<surface_intersection> traceRay(glm::vec3 origin, glm::vec3 direction) const override;
};

#endif // GEOMETRYFLOOR_H
