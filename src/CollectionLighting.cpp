#include "CollectionLighting.h"

#include "lighting/lighting.h"

#include "sampling/ddf.h"

#include <glm/vec3.hpp>

using namespace glm;
using namespace std;

shared_ptr<const Ddf> CollectionLighting::distributionInPoint(glm::vec3 pos) const {
    std::shared_ptr<const Ddf> res = unite();
    for(shared_ptr<const Light> l: lights){
        res = unite(res, l->lightToPoint(pos));
    }
    return res;
}

optional<light_intersection> CollectionLighting::traceRayToLight(glm::vec3 origin, glm::vec3 direction) const {
    optional<light_intersection> res;
    for(shared_ptr<const Light> l: lights){
        optional<light_intersection> e = l->traceRay(origin, direction);
        if(!e.has_value())
            continue;
        if(!res.has_value() || length(res->position-origin) > length(e->position-origin))
            res = e;
    }

    return res;
}

void CollectionLighting::addPointLight(vec3 position, float power){
    lights.push_back(make_shared<const PointLight>(position, power));
}
void CollectionLighting::addSphereLight(vec3 position, float radius, float power){
    lights.push_back(make_shared<const SphereLight>(position, radius, power));
}
void CollectionLighting::addAreaLight(vec3 corner, vec3 normal, vec3 x_side, float power){
    vec3 y_side = cross(normal, x_side);
    shared_ptr<const Light> light = make_shared<const AreaLight>(corner, x_side, y_side, power);
    lights.push_back(light);
}
