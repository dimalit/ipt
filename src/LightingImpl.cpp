#include "LightingImpl.h"

#include "sampling.h"

using namespace std;

shared_ptr<const Ddf> LightingImpl::distributionInPoint(glm::vec3 pos) const {
    std::shared_ptr<const Ddf> res;
    for(shared_ptr<const Light> l: lights){
        res = unite(res, l->lightToPoint(pos));
    }
    return res;
}

optional<light_intersection> LightingImpl::traceRayToLight(glm::vec3 origin, glm::vec3 direction) const {
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
