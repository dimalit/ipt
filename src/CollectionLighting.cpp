#include "CollectionLighting.h"

#include "lighting/lighting.h"

#include <libddf/ddf.h>

#include <glm/vec3.hpp>

using namespace glm;
using namespace std;

unique_ptr<Ddf> CollectionLighting::distributionInPoint(glm::vec3 pos) const {
    std::unique_ptr<Ddf> res = unite();
    float acc_power = 0.0f;
    for(const shared_ptr<const Light>& l: lights){
        unique_ptr<Ddf> light_ddf = l->lightToPoint(pos);
        res = unite(move(res), acc_power, move(light_ddf), l->power);
        acc_power += l->power;
    }
    return res;
}

optional<light_intersection> CollectionLighting::traceRayToLight(glm::vec3 origin, glm::vec3 direction) const {
    optional<light_intersection> res;
    for(const shared_ptr<const Light>& l: lights){
        optional<light_intersection> e = l->traceRay(origin, direction);
        if(!e.has_value())
            continue;
        if(!res.has_value() || length(res->position-origin) > length(e->position-origin))
            res = e;
    }

    return res;
}

void CollectionLighting::addPointLight(vec3 position, float virtual_radius, float power){
    lights.push_back(make_shared<const PointLight>(position, virtual_radius, power));
}
void CollectionLighting::addSphereLight(vec3 position, float radius, float power){
    lights.push_back(make_shared<const SphereLight>(position, radius, power));
}
void CollectionLighting::addSquareLight(vec3 corner, vec3 normal, vec3 x_side, float power){
    vec3 y_side = cross(normal, x_side);
    shared_ptr<const Light> light = make_shared<const AreaLight>(corner, x_side, y_side, power);
    lights.push_back(light);
}
void CollectionLighting::addTriangleLight(glm::vec3 corner, glm::vec3 x_side, glm::vec3 y_side, float power){
    shared_ptr<const Light> light = make_shared<const AreaLight>(corner, x_side, y_side, power, AreaLight::TYPE_TRIANLE);
    lights.push_back(light);
}

void CollectionLighting::addOuterLight(float radius, float power){
    shared_ptr<const Light> light = make_shared<InvertedSphereLight>(vec3(), radius, power);
    lights.push_back(light);
}
