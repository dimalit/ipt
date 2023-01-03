#include "lighting.h"

#include "geometric_utils.h"

#include <random>

using namespace glm;
using namespace std;

// TODO deduplicate
static struct{
    std::random_device rdev;
    std::mt19937 gen = std::mt19937(rdev());

    std::uniform_real_distribution<> dist = std::uniform_real_distribution<>(0.0f, 1.0f);

    float operator()() {
        return dist(gen);
    }
} randf;

AreaLight::AreaLight(vec3 origin, vec3 x_axis, vec3 y_axis, float w, type_t type){
    this->origin = origin;
    this->x_axis = x_axis;
    this->y_axis = y_axis;
    this->weight = w;
    this->type = type;
}
std::optional<light_intersection> AreaLight::trySample() const {
    float u1 = randf();
    float u2 = randf() * (type==TYPE_TRIANLE ? 1.0f-u1 : 1.0f );
    vec3 pos = x_axis*u1 + y_axis*u2;

    light_intersection res;
    res.position = pos;
    res.radiation = weight / area();

    return res;
}

std::optional<light_intersection> AreaLight::traceRay(vec3 origin, vec3 direction) const {
    // <origin+dir*t-this->origin, n>=0
    // origin*n + t*dir*n - this->origin*n = 0
    // t = n*(this->origin-origin) / n*dir
    vec3 n = normalize(cross(x_axis, y_axis));
    float n_dir = dot(n, direction);
    if(n_dir < 1e-6)
        return {};
    float t = dot(n, this->origin-origin) / n_dir;
    if(t<0.0f)
        return {};
    vec3 pos = origin + direction*t;

    float cx = dot(x_axis, pos-origin);
    float cy = dot(y_axis, pos-origin);

    bool hit = false;
    if(type == TYPE_DIAMOND)
        hit = cx >= 0.0f && cx <= 1.0f && cy >= 0.0f && cy <= 1.0f;
    else if(type == TYPE_TRIANLE)
        hit = cx >= 0.0f && cy >= 0 && cx+cy <= 1.0f;
    else {
        assert(false);
    }

    if(!hit)
        return {};

    light_intersection res;
    res.eye_ray = direction;
    res.position = pos;
    res.radiation = weight / area();

    return res;
}

optional<light_intersection> SphereLight::traceRay(glm::vec3 origin, glm::vec3 direction) const {
    float t = intersection_with_sphere(radius, origin, direction);
    if(t == std::numeric_limits<float>::infinity())
        return {};

    light_intersection res;
    res.eye_ray = direction;
    res.position = origin + direction*t;

    vec3 normal = normalize(res.position - this->origin);
    float cosinus = dot(normal, -direction);
    res.radiation = weight / 4.0 / M_PI * cosinus;

    return res;
}

optional<light_intersection> SphereLight::trySample() const {

    float u1 = randf()*2.0f - 1.0f;     // -1..+1
    float u2 = randf();
    float alpha = acos(u1);
    float phi = 2*M_PI*u2;
    float r = sin(alpha);
    vec3 pos(r*cos(phi), r*sin(phi), u1);

    // TODO no direction information here!
    light_intersection res;
    res.position = pos;
    res.radiation = weight / 4.0 / M_PI;    // no cosinus!

    return res;
}
