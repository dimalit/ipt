#include "lighting.h"

#include <randf.h>

using namespace glm;
using namespace std;

light_intersection Lighting::last_sample;

// NB Although this is duplicated code, it allows to break dependency from main project
static float intersection_with_sphere(float radius, vec3 origin, vec3 direction){
    //|origin + direction*t| = radius
    //(o+d*t)*(o+d*t)=r^2
    // o^2 + 2*o*d*t + d^2*t^2 = r^2
    // desc = (2od)^2 - 4*d^2*(o^2-r^2)
    // res = (-2od +- sqrt(desc)) / (2d^2)
    float desc = 4.0f*pow(dot(origin, direction), 2.0f) - 4.0f*dot(direction,direction)*(dot(origin,origin)-radius*radius);
    if(desc < 0.0f)
        return std::numeric_limits<float>::infinity();
    float t1 = (-2.0*dot(origin, direction) - sqrt(desc)) / 2.0 / dot(direction, direction);
    float t2 = (-2.0*dot(origin, direction) + sqrt(desc)) / 2.0 / dot(direction, direction);
    if(t1<1e-6)
        t1 = std::numeric_limits<float>::infinity();
    if(t2<1e-6)
        t2 = std::numeric_limits<float>::infinity();
    float t = std::min(t1, t2);

    // discard back-face intersections
    vec3 pos = origin + direction*t;
    vec3 outer_normal = normalize(pos);
    float direction_sign = dot(outer_normal, origin-pos);   // >0 for outer rays
    float position_sign  = length(origin) - radius;         // >0 for outer rays
    if( direction_sign * position_sign <= 0.0f)
        return std::numeric_limits<float>::infinity();
    return t;
}

class LightToDistribution: public Ddf {
    const Light* light;
    glm::vec3 origin;
public:
    LightToDistribution(const Light* light, glm::vec3 origin) {
        this->light = light;
        this->origin = origin;
        // TODO check specific light types elsewhere!
        if(dynamic_cast<const PointLight*>(light)){
            this->max_value = std::numeric_limits<float>::infinity();
        }
    }
    virtual glm::vec3 trySample() const override;
    virtual float value( glm::vec3 direction ) const override;
};

// filters samples by it's own PDF - resulting in a flat PDF
class LightToChainedDistribution: public Ddf {
    shared_ptr<const LightToDistribution> backend_disribution;
public:
    LightToChainedDistribution(const Light* light, glm::vec3 origin)
        :backend_disribution(make_shared<LightToDistribution>(light, origin)){
            vec3 nearest_point = light->nearestPointTo(origin);
            this->max_value = nearest_point == vec3() ? 0.0f : backend_disribution->value(normalize(nearest_point-origin));
    }
    virtual glm::vec3 trySample() const override;
    virtual float value( glm::vec3 direction ) const override;
};

glm::vec3 LightToDistribution::trySample() const {
    vec3 dir;
    light_intersection inter = light->sample();
    dir = normalize(inter.position-origin);
    float cosinus = dot(inter.normal, -dir);
    if(cosinus < 1e-5f)               // if facing back
        return vec3();
    Lighting::last_sample = inter;  // HACK TODO how to make it accessible in a cleaner way?
    return dir;
}

float LightToDistribution::value( glm::vec3 direction ) const {
    // HACK
    std::optional<light_intersection> inter = direction==vec3() ? Lighting::last_sample : light->traceRay(origin, direction);
    if(!inter.has_value())
        return 0.0f;
    glm::vec3 dir = glm::normalize(inter->position-origin);
    float cosinus = dot(inter->normal, -dir);
    assert(cosinus > -1e-5);
    if(cosinus<0.0f)
        return 0.0f;
    float decay = dot(inter->position-origin, inter->position-origin);
    return decay/cosinus/light->area;
}

static bool p_hit(float prob){
    // TODO this 1.3 is obviously a bug
    //assert(prob >= 0.0f && prob < 1.1f);
    return randf() < prob;
}

glm::vec3 LightToChainedDistribution::trySample() const {
    vec3 x = backend_disribution->trySample();
    if(x == vec3())         // fail if fail
        return x;

    float backend_value = backend_disribution->value( x );
    bool hit = p_hit( max_value / backend_value );

    return hit ? x : vec3();
}

float LightToChainedDistribution::value( glm::vec3 direction ) const {
    if(backend_disribution->value(direction) != 0.0f)
        return max_value;
    else
        return 0.0f;
}

std::shared_ptr<const Ddf> Light::lightToPoint(glm::vec3 pos) const {
    return std::make_shared<const LightToDistribution>(this, pos);
}

AreaLight::AreaLight(vec3 origin, vec3 x_axis, vec3 y_axis, float power, type_t type){
    this->position = origin;
    this->x_axis = x_axis;
    this->y_axis = y_axis;
    this->power = power;
    this->type = type;
    float full_area = length(cross(x_axis, y_axis));
    area = type==TYPE_DIAMOND ? full_area : full_area/2.0f;
}

// TODO Maybe use here sampling parameters to adjust surface_power?
light_intersection AreaLight::sample() const {
    float u1 = randf();
    float u2 = randf() * (type==TYPE_TRIANLE ? 1.0f-u1 : 1.0f );
    vec3 pos = x_axis*u1 + y_axis*u2;

    light_intersection res;
    res.position = pos + this->position;
    res.normal = normalize(cross(x_axis, y_axis));
    res.surface_power = this->power/this->area;

    return res;
}

// TODO Test and repair it in case ray lies in the plane of this light source
std::optional<light_intersection> AreaLight::traceRay(vec3 origin, vec3 direction) const {    
    // <origin+dir*t-this->origin, n>=0
    // origin*n + t*dir*n - this->origin*n = 0
    // t = n*(this->origin-origin) / n*dir
    vec3 n = normalize(cross(x_axis, y_axis));

    float n_dir = dot(n, direction);

    // skip extreme angles and back-face hits
    if(abs(n_dir) < 1e-6 || n_dir > 0.0f)
        return {};

    float t = dot(n, this->position-origin) / n_dir;
    if(t<1e-6)
        return {};
    vec3 pos = origin + direction*t;

    float cx = dot(normalize(x_axis), pos-this->position) / length(x_axis);
    float cy = dot(normalize(y_axis), pos-this->position) / length(y_axis);

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
    res.position = pos;
    res.normal = normalize(cross(x_axis, y_axis));
    res.surface_power = this->power/this->area;

    return res;
}

vec3 nearest_line_point(vec3 a, vec3 b, vec3 poi){
    vec3 a_to_poi = poi - a;
    vec3 unit_dir = normalize(b-a);
    float coord = dot(a_to_poi, unit_dir);
    if(coord < 0.0f)
        return a;
    else if(coord > length(b-a))
        return b;
    else
        return a+unit_dir*coord;
}

vec3 AreaLight::nearestPointTo(vec3 point) const {

    float max_influence = 0.0f;
    vec3 res = vec3();

    // TODO how to handle case when point is behind light direction?
    vec3 normal = normalize(cross(x_axis, y_axis));
    optional<light_intersection> inter = traceRay(point, -normal);
    // ignore back and near-same-plane points
    // TODO systematically handle such cases!
    if(dot(normal, normalize(point-position))<1e-6)
        return vec3();

    float min_distance = numeric_limits<float>::infinity();

    if(inter.has_value()){
        min_distance = length(inter->position-point);
        res = inter->position;
    }

    vec3 p = nearest_line_point(position, position+x_axis, point);
    float distance = length(p-point);
    if(min_distance > distance){
        min_distance = distance;
        res = p;
    }

    p = nearest_line_point(position, position+y_axis, point);
    distance = length(p-point);
    if(min_distance > distance){
        min_distance = distance;
        res = p;
    }

    if(type==TYPE_TRIANLE){
        p = nearest_line_point(position+x_axis, position+y_axis, point);
        distance = length(p-point);
        if(min_distance > distance){
            min_distance = distance;
            res = p;
        }
    }// if triangle
    else{
        p = nearest_line_point(position+x_axis, position+x_axis+y_axis, point);
        distance = length(p-point);
        if(min_distance > distance){
            min_distance = distance;
            res = p;
        }
        p = nearest_line_point(position+y_axis, position+x_axis+y_axis, point);
        distance = length(p-point);
        if(min_distance > distance){
            min_distance = distance;
            res = p;
        }
    }

    // adjst not to be on the very edge
    vec3 inside_point = position+(x_axis+y_axis)/4.0f;
    vec3 to_inside = inside_point - res;
    res += to_inside / 100.0f;

    return res;
}

optional<light_intersection> SphereLight::traceRay(glm::vec3 origin, glm::vec3 direction) const {
    float t = intersection_with_sphere(radius, origin-this->position, direction);
    if(t == std::numeric_limits<float>::infinity())
        return {};

    light_intersection res;
    res.position = origin + direction*t;
    res.normal = normalize(res.position - this->position);
    res.surface_power = this->power/this->area;

    return res;
}

// TODO Maybe use here sampling parameters to adjust surface_power?
light_intersection SphereLight::sample() const {

    // TODO Use here SphericalDdf?
    float u1 = randf()*2.0f - 1.0f;     // -1..+1
    float u2 = randf();
    float alpha = acos(u1);
    float phi = 2*M_PI*u2;
    float r = radius*sin(alpha);
    vec3 pos(r*cos(phi), r*sin(phi), radius*u1);

    light_intersection res;
    res.position = pos + this->position;
    res.normal = normalize(pos);
    res.surface_power = this->power/this->area;

    return res;
}

// TODO Maybe use here sampling parameters to adjust surface_power?
light_intersection PointLight::sample() const {

    // TODO deduplicate with SphereLight
    float u1 = randf()*2.0f - 1.0f;     // -1..+1
    float u2 = randf();
    float alpha = acos(u1);
    float phi = 2*M_PI*u2;
    float r = sin(alpha);
    vec3 normal(r*cos(phi), r*sin(phi), u1);

    light_intersection res;
    res.position = this->position;
    res.normal = normal;
    res.surface_power = std::numeric_limits<float>::signaling_NaN();

    return res;
}
