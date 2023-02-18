#ifndef GEOMETRYSMALLPT_H
#define GEOMETRYSMALLPT_H

#include <tracer_interfaces.h>

struct GeometrySmallPt: public Geometry{
    virtual std::optional<surface_intersection> traceRay(glm::vec3 origin, glm::vec3 direction) const override;
};

#endif // GEOMETRYSMALLPT_H
