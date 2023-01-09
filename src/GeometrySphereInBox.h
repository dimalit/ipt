#include "tracer_interfaces.h"

struct GeometrySphereInBox: public Geometry {
    virtual std::optional<surface_intersection> traceRay(glm::vec3 origin, glm::vec3 direction) const override;
};
