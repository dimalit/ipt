#include "GeometryFloor.h"

#include "geometric_utils.h"

#include <sampling/sampling.h>

using namespace glm;
using namespace std;

std::optional<surface_intersection> GeometryFloor::traceRay(vec3 origin, vec3 direction) const {
    float t = intersection_with_box_plane({0,0,-1}, origin, direction);
    if(t==std::numeric_limits<float>::infinity() || t<0.0f || abs(t)<1e-6)
        return {};

    surface_intersection res;
    res.position = origin + direction * t;
    res.curvature = 0.0f;
    res.normal = {0,0,1};
    res.sdf = make_shared<const CosineDdf>();
    //res.sdf = unite(make_shared<const CosineDdf>(0.8f), make_shared<const MirrorDdf>(0.2f));

    return res;
}
