#include "geometry.h"

#include "sampling.h"

#include "geometric_utils.h"

using namespace glm;
using namespace std;

std::optional<surface_intersection> Floor::traceRay(vec3 origin, vec3 direction) const {
    float t = intersection_with_box_plane({0,0,-1}, origin, direction);
    if(t==std::numeric_limits<float>::infinity() || t<0.0f || abs(t)<1e-6)
        return {};

    surface_intersection res;
    res.position = origin + direction * t;
    res.curvature = 0.0f;
    res.normal = {0,0,1};
    res.sdf = make_shared<const CosineDdf>(1.0f);
    //res.sdf = unite(make_shared<const CosineDdf>(0.8f), make_shared<const MirrorDdf>(0.2f));

    return res;
}

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
        if(t>1e-6 && t<dist){
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

    if(dist==numeric_limits<float>::infinity())
        return {};

    surface_intersection res;
    res.position = origin + direction * dist;

    if(intersected_plane >= 0){
        res.curvature = 0.0f;

        // material
        res.normal = -planes[intersected_plane];   // inside

        shared_ptr<const Ddf> dis = make_shared<const CosineDdf>(1.0f);

        shared_ptr<RotateDdf> rotate = make_shared<RotateDdf>(dis, res.normal);
        res.sdf = rotate;
    }
    else if(intersected_sphere){
        res.curvature = 1.0f;

        // material
        res.normal = res.position;
        vec3 reflection = reflect(direction, res.normal);
        float eye_angle_cos = dot(-direction, res.normal);

        shared_ptr<const Ddf> diffuse = make_shared<const CosineDdf>(0.8f*eye_angle_cos);
        // DEBUG temporary skip: shared_ptr<const Ddf> specular = make_shared<const MirrorDdf>(0.2f);

        shared_ptr<RotateDdf> rotate_diffuse = make_shared<RotateDdf>(diffuse, res.normal);

        //shared_ptr<RotateDdf> rotate_specular = make_shared<RotateDdf>(reflection);
        //rotate_specular->origin = specular;
        shared_ptr<RotateDdf> rotate_cap = make_shared<RotateDdf>(make_shared<UpperHalfDdf>(), res.normal);

        //shared_ptr<const Ddf> dist = unite(rotate_diffuse, ::apply(rotate_specular, rotate_cap));

        res.sdf = rotate_diffuse;
    }
    else {
        return {};
    }

    return res;
}
