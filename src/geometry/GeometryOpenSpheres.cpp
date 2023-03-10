#include "GeometryOpenSpheres.h"

#include "GeometryOpenSpheres.h"

#include "geometric_utils.h"

#include <libddf/ddf_detail.h>

using namespace glm;
using namespace std;

std::optional<surface_intersection> GeometryOpenSpheres::traceRay(vec3 origin, vec3 direction) const {
    vec3 spheres[] = {
        {-0.2f, -0.2f, -0.8f},
        {+0.2f, -0.2f, -0.8f},
        { 0.0f, +0.2f, -0.8f}
    };

    float dist = std::numeric_limits<float>::infinity();
    bool intersected_plane = false;
    int intersected_sphere = -1;

    for(size_t i = 0; i<3; ++i){
        float t = intersection_with_sphere(0.2f, origin-spheres[i], direction);
        if(t<dist){
            dist = t;
            intersected_sphere = i;
        }
    }// for i

    float t = intersection_with_box_plane({0,0,-1}, origin, direction);
    if(t<dist){
        dist = t;
        intersected_plane = true;
        intersected_sphere = -1;
    }

    if(dist==numeric_limits<float>::infinity())
        return {};

    surface_intersection res;
    res.position = origin + direction * dist;

    if(intersected_plane){
        res.curvature = 0.0f;

        // material
        res.normal = {0,0,1};
        unique_ptr<Ddf> dis = make_unique<CosineDdf>();
        res.sdf = move(dis);
    }
    else if(intersected_sphere >= 0){
        res.curvature = 1.0/0.2f;

        // material
        res.normal = normalize(res.position-spheres[intersected_sphere]);
        float eye_angle_cos = dot(-direction, res.normal);

        unique_ptr<Ddf> diffuse = make_unique<CosineDdf>();
        unique_ptr<RotateDdf> rotate_diffuse = make_unique<RotateDdf>(move(diffuse), res.normal);
        res.sdf = move(rotate_diffuse);
    }
    else {
        return {};
    }

    return move(res);
}
