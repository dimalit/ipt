#ifndef LIGHTINGIMPL_H
#define LIGHTINGIMPL_H

#include "lighting.h"

#include <vector>

struct LightingImpl: public Lighting {
    std::vector<std::shared_ptr<const LightImpl>> lights;
    virtual std::shared_ptr<const Ddf> distributionInPoint(glm::vec3 pos) const;
    virtual std::optional<light_intersection> traceRayToLight(glm::vec3 origin, glm::vec3 direction) const;
};

#endif // LIGHTINGIMPL_H
