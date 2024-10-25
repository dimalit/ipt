#ifndef GEOMETRYCORNER_H
#define GEOMETRYCORNER_H

#include <tracer_interfaces.h>

// x,y,z = -1 -> 1 corner
struct GeometryCorner: public Geometry {
    virtual std::optional<surface_intersection> traceRay(glm::vec3 origin, glm::vec3 direction) const override;
};

#endif // GEOMETRYCORNER_H
