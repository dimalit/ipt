#ifndef LIGHTINGIMPL_H
#define LIGHTINGIMPL_H

#include "tracer_interfaces.h"

#include <memory>
#include <optional>

#include <vector>

class Light;

struct LightingImpl: public Lighting {
    std::vector<std::shared_ptr<const Light>> lights;
    virtual std::shared_ptr<const Ddf> distributionInPoint(glm::vec3 pos) const;
    virtual std::optional<light_intersection> traceRayToLight(glm::vec3 origin, glm::vec3 direction) const;

    void addPointLight(glm::vec3 position, float power=1.0f);
    void addSphereLight(glm::vec3 position, float radius, float power=1.0f);
    void addAreaLight(glm::vec3 corner, glm::vec3 normal, glm::vec3 x_side, float power=1.0f);
};

#endif // LIGHTINGIMPL_H
