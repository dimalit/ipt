#include "CollectionLighting.h"

#include "lighting/lighting.h"

#include <libddf/ddf.h>

#include <glm/vec3.hpp>

using namespace glm;
using namespace std;

shared_ptr<const Ddf> CollectionLighting::distributionInPoint(glm::vec3 pos) const {
    std::shared_ptr<const Ddf> res = unite();
    float acc_power = 0.0f;
    for(shared_ptr<const Light> l: lights){
        shared_ptr<const Ddf> light_ddf = l->lightToPoint(pos);
//        vec3 nearest_point = l->nearestPointTo(pos);
//        float light_ddf_min = light_ddf->value(normalize(nearest_point-pos));
//        shared_ptr<const Ddf> mix = chain(light_ddf, make_shared<InvertDdf>(light_ddf, light_ddf_min));
        // TODO max_value==0 means light cannot shine here: design this in a better way!
        if(light_ddf->max_value == 0.0f)
            continue;
        res = unite(res, acc_power, light_ddf, l->power);
        acc_power += l->power;
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

void CollectionLighting::addPointLight(vec3 position, float virtual_radius, float power){
    lights.push_back(make_shared<const PointLight>(position, virtual_radius, power));
}
void CollectionLighting::addSphereLight(vec3 position, float radius, float power){
    lights.push_back(make_shared<const SphereLight>(position, radius, power));
}
void CollectionLighting::addAreaLight(vec3 corner, vec3 normal, vec3 x_side, float power){
    vec3 y_side = cross(normal, x_side);
    shared_ptr<const Light> light = make_shared<const AreaLight>(corner, x_side, y_side, power);
    lights.push_back(light);
}

void CollectionLighting::addOuterLight(float radius, float power){
    shared_ptr<const Light> light = make_shared<InvertedSphereLight>(vec3(), radius, power);
    lights.push_back(light);
}
