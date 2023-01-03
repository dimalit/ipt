#include "geometry.h"

#include "sampling.h"

#include "geometric_utils.h"

using namespace glm;
using namespace std;

std::optional<surface_intersection> SphereInBox::traceRay(vec3 origin, vec3 direction) const {
    vec3 planes[] = {
        {1,0,0},
        {0,1,0},
        {0,0,1},
        {-1,0,0},
        {0,-1,0},
        {0,0,-1}
    };

    float dist = std::numeric_limits<float>::infinity();
    size_t intersected_plane = -1;
    bool intersected_sphere = false;

    for(size_t i = 0; i<6; ++i){
        float t = intersection_with_box_plane(planes[i], origin, direction);
        if(t>0.0f && t<dist){
            dist = t;
            intersected_plane = i;
        }
    }// for i

    float t = intersection_with_sphere(1.0f, origin, direction);
    if(t<dist){
        dist = t;
        intersected_sphere = true;
        intersected_plane = -1;
    }

    surface_intersection res;
    res.eye_ray = direction;
    res.position = origin + direction * dist;

    if(intersected_plane >= 0){
        res.curvature = 0.0f;

        // material
        vec3 normal = -planes[intersected_plane];   // inside

        shared_ptr<const Distribution> dis = make_shared<const Cosine>(1.0f);

        shared_ptr<Rotate> rotate = make_shared<Rotate>(normal);
        rotate->origin = dis;
        res.sdf = rotate;
    }
    else if(intersected_sphere){
        res.curvature = 1.0f;

        // material
        vec3 normal = res.position;
        vec3 reflection = reflect(direction, normal);
        float eye_angle_cos = dot(-direction, normal);

        shared_ptr<const Distribution> diffuse = make_shared<const Cosine>(0.8f*eye_angle_cos);
        shared_ptr<const Distribution> specular = make_shared<const Mirrorly>(0.2f);

        shared_ptr<Rotate> rotate_diffuse = make_shared<Rotate>(normal);
        rotate_diffuse->origin = diffuse;

        shared_ptr<Rotate> rotate_specular = make_shared<Rotate>(reflection);
        rotate_specular->origin = specular;
        shared_ptr<Rotate> rotate_cap = make_shared<Rotate>(normal);
        rotate_cap->origin = make_shared<UpperHalf>();

        shared_ptr<const Distribution> dist = unite(rotate_diffuse, apply(rotate_specular, rotate_cap));

        res.sdf = dist;
    }
    else {
        return {};
    }

    return res;
}
