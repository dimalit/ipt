#ifndef FRACTALSPHERES_H
#define FRACTALSPHERES_H

#include <tracer_interfaces.h>

#include <vector>

class FractalSpheres : public Geometry
{
public:
    FractalSpheres();
    virtual std::optional<surface_intersection> traceRay(glm::vec3 origin, glm::vec3 direction) const override;
private:
    std::vector<float> rs;
    std::vector<glm::vec3> cs;
};

#endif // FRACTALSPHERES_H
