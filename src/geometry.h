#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "tracer_interfaces.h"

struct SphereInBox: public Geometry {
    virtual std::optional<surface_intersection> traceRay(glm::vec3 origin, glm::vec3 direction) const override;
};

struct Floor: public Geometry {
    virtual std::optional<surface_intersection> traceRay(glm::vec3 origin, glm::vec3 direction) const override;
};

#endif // GEOMETRY_H
