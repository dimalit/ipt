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
    // Finds where given ray intersects with the geometry
    virtual std::optional<surface_intersection> traceRay(glm::vec3 origin, glm::vec3 direction) const = 0;
};

struct Lighting {
    // Returs DDF of all lights acting on given point
    virtual std::unique_ptr<Ddf> distributionInPoint(glm::vec3 pos) const = 0;
    // Finds intersection of given ray with lights, or absense of thereof
    virtual std::optional<light_intersection> traceRayToLight(glm::vec3 origin, glm::vec3 direction) const = 0;
    static light_intersection last_sample;
};

struct Camera {
    // Returns ray that corresponds to position x,y within the frame, x,y = 0..1
    // Can use random numbers for repeating calls with the same x,y
    virtual std::pair<glm::vec3, glm::vec3> sampleRay(float x, float y) const = 0;
};

struct Scene{
    std::shared_ptr<const Geometry> geometry;
    std::shared_ptr<const Lighting> lighting;
    std::shared_ptr<const Camera> camera;
};

struct RenderPlane {
    // Accumulates rays from many calls, x,y = 0..1
    virtual void addRay(float x, float y, float value) = 0;
};

#endif // TRACER_INTERFACES_H
