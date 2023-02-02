#include "geometric_utils.h"

#include <glm/geometric.hpp>

using namespace glm;

// plane 0,1,0 0,0,-1 etc
float intersection_with_box_plane(vec3 plane, vec3 origin, vec3 direction){
    //(origin + direction*t - plane) * plane = 0
    //(origin + direction*t) * plane = 1
    //direction*plane * t = 1 - origin*plane
    // t = (1-origin*plane)/(direction*plane)
    float dir_plane = dot(direction, plane);
    if(abs(dir_plane) < 1e-6)
            return std::numeric_limits<float>::infinity();
    float t = (1.0f - dot(origin, plane)) / dir_plane;
    vec3 point = origin + direction * t;
    if( abs(point.x) > 1.0f || abs(point.y) > 1.0f || abs(point.z) > 1.0f )
        return std::numeric_limits<float>::infinity();
    // exclude back face
    if(dot(direction, plane) < 0.0f)
        return std::numeric_limits<float>::infinity();
    if(t<1e-6)
        return std::numeric_limits<float>::infinity();
    return t;
}

float intersection_with_sphere(float radius, vec3 origin, vec3 direction){
    //|origin + direction*t| = radius
    //(o+d*t)*(o+d*t)=r^2
    // o^2 + 2*o*d*t + d^2*t^2 = r^2
    // desc = (2od)^2 - 4*d^2*(o^2-r^2)
    // res = (-2od +- sqrt(desc)) / (2d^2)

    // pre-compute common values
    float origin_x_dir = dot(origin, direction);
    // end

    float desc = 4.0f*pow(origin_x_dir, 2.0f) - 4.0f*(dot(origin,origin)-radius*radius);
    if(desc < 0.0f)
        return std::numeric_limits<float>::infinity();
    float sqrt_desc = sqrt(desc);
    float t1 = (-2.0*origin_x_dir - sqrt_desc) / 2.0;
    float t2 = (-2.0*origin_x_dir + sqrt_desc) / 2.0;
    if(t1<1e-6)
        t1 = std::numeric_limits<float>::infinity();
    if(t2<1e-6)
        t2 = std::numeric_limits<float>::infinity();
    float t = std::min(t1, t2);

    vec3 pos = origin + direction*t;
    if(dot(pos, origin-pos) <= 0.0f)
        return std::numeric_limits<float>::infinity();
    return t;
}

// TODO Probably not needed
float rays_distance(vec3 o1, vec3 d1, vec3 o2, vec3 d2){
    vec3 crs = cross(d1, d2);
    return abs(dot(o2-o1, crs)) / length(crs);
}

vec3 ray2_closest_point(vec3 o1, vec3 d1, vec3 o2, vec3 d2){
    // TODO Pretend we implemented it
    assert(false && "Not implemented");
    return o1+d1+o2+d2;
}
