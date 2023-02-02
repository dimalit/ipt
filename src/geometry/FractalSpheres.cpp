#include "FractalSpheres.h"

#include "geometric_utils.h"

#include <libddf/ddf_detail.h>

#include <glm/geometric.hpp>

#include <cmath>
#include <functional>
#include <iostream>

using namespace glm;
using namespace std;

void generate_spheres(float r1, vec3 c1, float r2, vec3 c2, function<bool(float r, vec3 c)> callback){
    float r3;
    vec3 c3;

    float L = length(c1-c2)-r1-r2;
    if(L<0.01)
        return;

    float sin_alpha = r1/(r1+L);
    float alpha = asin(sin_alpha);
    float sin_beta = r2/(r2+L);
    float beta  = asin(sin_beta);
    float gamma = M_PIf32 - alpha - beta;
    assert(gamma > 0.0f);

    float A=L*sin_alpha/sin(gamma);
    float x = A*sin(gamma/2)/sin(M_PIf32-beta-gamma/2);

    c3 = c1+normalize(c2-c1)*(x+r1);
    r3 = x*sin_beta;

    if(callback(r3, c3))
        return;
    generate_spheres(r1, c1, r3, c3, callback);
    generate_spheres(r3, c3, r2, c2, callback);
}

optional<surface_intersection> FractalSpheres::traceRay(vec3 origin, vec3 direction) const {
    float r1 = 1.0f;
    vec3 c1 = vec3(-2,0,0);
    float r2 = 1.0f;
    vec3 c2 = vec3(2,0,0);

    float res_r;
    vec3 res_c;
    float res_dist = numeric_limits<float>::infinity();

    auto sphere_callback = [origin, direction, &res_r, &res_c, &res_dist](float r, vec3 c)->bool {

        if(r<0.01)
            return true;

        //cout << r << " " << c.x << endl;

        float dist = intersection_with_sphere(r, origin-c, direction);
        if(isfinite(dist) && std::abs(dist)>1e-6 && dist < res_dist){
            res_r = r;
            res_c = c;
            res_dist = dist;
        }// if intersect
        return false;
    };

    sphere_callback(r1, c1);
    sphere_callback(r2, c2);
    generate_spheres(r1, c1, r2, c2, sphere_callback);

    if(!isfinite(res_dist))
        return {};

    surface_intersection res;
    res.position = origin + direction*res_dist;
    res.normal = normalize(res.position - res_c);
    res.albedo = 1.0f;
    res.curvature = 1.0f/res_r;
    res.sdf = make_shared<RotateDdf>(make_shared<CosineDdf>(), res.normal);

    return res;
}
