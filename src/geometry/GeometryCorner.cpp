#include "GeometryCorner.h"

#include "geometric_utils.h"

#include <libddf/ddf_detail.h>

using namespace glm;
using namespace std;

std::optional<surface_intersection> GeometryCorner::traceRay(vec3 origin, vec3 direction) const {
    float tx = intersection_with_box_plane({-1,0,0}, origin, direction);
    float ty = intersection_with_box_plane({0,-1,0}, origin, direction);
    float tz = intersection_with_box_plane({0,0,-1}, origin, direction);

    float t = tx;
    vec3 normal = {1,0,0};
    if(ty<t){
        t = ty;
        normal = {0,1,0};
    }
    if(tz<t){
        t = tz;
        normal = {0,0,1}        ;
    }

    if(t==std::numeric_limits<float>::infinity() || t<0.0f || abs(t)<1e-6)
        return {};

    unique_ptr<RotateDdf> rotated_sdf = make_unique<RotateDdf>(make_unique<CosineDdf>(), normal);

    surface_intersection res;
    res.position = origin + direction * t;
    res.curvature = 0.0f;
    res.normal = normal;
    res.sdf = move(rotated_sdf);
    //res.sdf = unite(make_shared<const CosineDdf>(0.8f), make_shared<const MirrorDdf>(0.2f));

    return move(res);
}
