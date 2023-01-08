#ifndef TRACER_INTERFACES_H
#define TRACER_INTERFACES_H

#include "sampling_fwd.h"

#include <glm/vec3.hpp>

#include <memory>
#include <optional>

struct intersection {
    glm::vec3 position;
    glm::vec3 normal;
    virtual ~intersection(){}
};

struct surface_intersection: public intersection {
    float curvature;		// my addition
    std::shared_ptr<const Ddf> sdf;
};

struct light_intersection: public intersection {
    float surface_power;
};

struct Geometry {
    virtual std::optional<surface_intersection> traceRay(glm::vec3 origin, glm::vec3 direction) const = 0;
};

struct Lighting {
    virtual std::shared_ptr<const Ddf> distributionInPoint(glm::vec3 pos) const = 0;
    virtual std::optional<light_intersection> traceRayToLight(glm::vec3 origin, glm::vec3 direction) const = 0;
};

struct Camera {
    virtual std::pair<glm::vec3, glm::vec3> sampleRay(float x, float y) const = 0;
};

struct Scene{
    std::shared_ptr<const Geometry> geometry;
    std::shared_ptr<const Lighting> lighting;
    std::shared_ptr<const Camera> camera;
};

struct RenderPlane {
    virtual void addRay(float x, float y, float value) = 0;
};

#endif // TRACER_INTERFACES_H
