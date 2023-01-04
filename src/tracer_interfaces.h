#ifndef TRACER_INTERFACES_H
#define TRACER_INTERFACES_H

#include "sampling_fwd.h"

#include <glm/vec3.hpp>

#include <memory>
#include <optional>

struct intersection {
    glm::vec3 position;
    glm::vec3 normal;
};

struct surface_intersection: public intersection {
    glm::vec3 eye_ray;
    float curvature;		// my addition
    std::shared_ptr<const Distribution> sdf;
};

struct light_intersection: public intersection {
};

struct Geometry {
    virtual std::optional<surface_intersection> traceRay(glm::vec3 origin, glm::vec3 direction) const = 0;
};

struct Lighting {
    virtual std::shared_ptr<const Distribution> distributionInPoint(glm::vec3 pos) const = 0;
    virtual std::optional<light_intersection> traceRayToLight(glm::vec3 origin, glm::vec3 direction) const = 0;
};

struct Camera {

};

struct Scene{
    std::shared_ptr<Geometry> geometry;
    std::shared_ptr<Lighting> lighting;
    std::shared_ptr<Camera> camera;
};

struct RenderPlane {

};

#endif // TRACER_INTERFACES_H
