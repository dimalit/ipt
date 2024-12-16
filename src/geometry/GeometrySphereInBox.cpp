#include "GeometrySphereInBox.h"

#include "geometric_utils.h"

#include <libddf/ddf_detail.h>

using namespace glm;
using namespace std;

std::optional<surface_intersection> GeometrySphereInBox::traceRay(vec3 origin, vec3 direction) const {
    vec3 planes[] = {
        {1,0,0},
        {0,1,0},
        {0,0,1},
        {-1,0,0},
        {0,0,-1}
    };

    float dist = std::numeric_limits<float>::infinity();
    int intersected_plane = -1;
    bool intersected_sphere = false;

    for(size_t i = 0; i<5; ++i){
        float t = intersection_with_box_plane(planes[i], origin, direction);
        if(t<dist){
            dist = t;
            intersected_plane = i;
        }
    }// for i

    float t = intersection_with_sphere(0.5f, origin, direction);
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

        unique_ptr<Ddf> dis = make_unique<CosineDdf>();

        unique_ptr<RotateDdf> rotate = make_unique<RotateDdf>(move(dis), res.normal);
        res.sdf = move(rotate);
    }
    else if(intersected_sphere){
        res.curvature = 2.0f;

        // material
        res.normal = normalize(res.position);
        vec3 reflection = reflect(direction, res.normal);
        float eye_angle_cos = dot(-direction, res.normal);

        unique_ptr<Ddf> diffuse = make_unique<CosineDdf>();
        // DEBUG temporary skip: shared_ptr<Ddf> specular = make_shared<const MirrorDdf>(0.2f);

        unique_ptr<RotateDdf> rotate_diffuse = make_unique<RotateDdf>(move(diffuse), res.normal);

        //shared_ptr<RotateDdf> rotate_specular = make_shared<RotateDdf>(reflection);
        //rotate_specular->origin = specular;
        unique_ptr<RotateDdf> rotate_cap = make_unique<RotateDdf>(make_unique<UpperHalfDdf>(), res.normal);

        //shared_ptr<Ddf> dist = unite(rotate_diffuse, ::apply(rotate_specular, rotate_cap));

        res.sdf = move(rotate_diffuse);
    }
    else {
        return {};
    }

    return move(res);
}
