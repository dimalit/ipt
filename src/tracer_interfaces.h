#ifndef TRACER_INTERFACES_H
#define TRACER_INTERFACES_H

#include "libddf/ddf.h"

#include <glm/vec3.hpp>

#include <memory>
#include <optional>

struct intersection {
    glm::vec3 position;
    glm::vec3 normal;
};

struct surface_intersection: public intersection {
    float curvature;		// my addition
    std::unique_ptr<Ddf> sdf;
    float albedo = 1.0f;
};

struct light_intersection: public intersection {
    float surface_power;
};

struct Geometry {
    virtual std::optional<surface_intersection> traceRay(glm::vec3 origin, glm::vec3 direction) const = 0;
};

struct Lighting {
    virtual std::unique_ptr<Ddf> distributionInPoint(glm::vec3 pos) const = 0;
    virtual std::optional<light_intersection> traceRayToLight(glm::vec3 origin, glm::vec3 direction) const = 0;
    static light_intersection last_sample;
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
