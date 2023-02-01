#ifndef FRACTALSPHERES_H
#define FRACTALSPHERES_H

#include <tracer_interfaces.h>

class FractalSpheres : public Geometry
{
    virtual std::optional<surface_intersection> traceRay(glm::vec3 origin, glm::vec3 direction) const override;
};

#endif // FRACTALSPHERES_H
