#ifndef COLLECTIONLIGHTING_H
#define COLLECTIONLIGHTING_H

#include "tracer_interfaces.h"

#include <vector>

class Light;

struct CollectionLighting: public Lighting {
    std::vector<std::shared_ptr<const Light>> lights;
    virtual std::shared_ptr<const Ddf> distributionInPoint(glm::vec3 pos) const;
    virtual std::optional<light_intersection> traceRayToLight(glm::vec3 origin, glm::vec3 direction) const;

    void addPointLight(glm::vec3 position, float virtual_radius, float power=1.0f);
    void addSphereLight(glm::vec3 position, float radius, float power=1.0f);
    void addAreaLight(glm::vec3 corner, glm::vec3 normal, glm::vec3 x_side, float power=1.0f);
};

#endif // COLLECTIONLIGHTING_H
