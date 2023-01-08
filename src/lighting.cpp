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
        float res = dist(gen);
        while(res==1.0f)
            res = dist(gen);
        return res;
    }
} randf;

class LightToDistribution: public DdfImpl {
    // TODO Think about this
    friend class Superposition;
private:
    const LightImpl* light;
    glm::vec3 origin;
public:
    LightToDistribution(const LightImpl* light, glm::vec3 origin) {
        this->full_theoretical_weight = light->power / pow(length(origin-light->position), 2);
        this->light = light;
        this->origin = origin;
    }
    virtual sample trySample() const override;
    virtual float value( glm::vec3 direction ) const override;
};

sample LightToDistribution::trySample() const {
    sample res;
    res.location = make_shared<light_intersection>(light->sample());
    res.direction = glm::normalize(res.location->position-origin);

    float cosinus = dot(res.location->normal, -res.direction);
    if(cosinus <= 0.0f)               // if facing back
        return sample();
    return randf() <= cosinus ? res : sample();
}

float LightToDistribution::value( glm::vec3 direction ) const {
    std::optional<light_intersection> inter = light->traceRay(origin, direction);
    if(!inter.has_value())
        return 0.0f;
    glm::vec3 dir = glm::normalize(inter->position-origin);
    float cosinus = dot(inter->normal, -dir);
    assert(cosinus > -1e-6);
    if(cosinus<0.0f)
        cosinus = 0.0f;
    return cosinus;
}

std::shared_ptr<const Ddf> Light::lightToPoint(glm::vec3 pos) const {
    const LightImpl* cast = dynamic_cast<const LightImpl*>(this);
    assert(cast);
    return std::make_shared<const LightToDistribution>(cast, pos);
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
light_intersection AreaLight::sample() const {
    float u1 = randf();
    float u2 = randf() * (type==TYPE_TRIANLE ? 1.0f-u1 : 1.0f );
    vec3 pos = x_axis*u1 + y_axis*u2;

    light_intersection res;
    res.position = pos + this->position;
    res.normal = normalize(cross(x_axis, y_axis));
    // TODO probably remove it somehow
    res.surface_power = std::numeric_limits<float>::signaling_NaN();

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

light_intersection SphereLight::sample() const {

    float u1 = randf()*2.0f - 1.0f;     // -1..+1
    float u2 = randf();
    float alpha = acos(u1);
    float phi = 2*M_PI*u2;
    float r = radius*sin(alpha);
    vec3 pos(r*cos(phi), r*sin(phi), radius*u1);

    light_intersection res;
    res.position = pos + this->position;
    res.normal = normalize(pos);
    // TODO probably remove it somehow
    res.surface_power = std::numeric_limits<float>::signaling_NaN();

    return res;
}

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
    // TODO probably remove it somehow
    res.surface_power = std::numeric_limits<float>::signaling_NaN();

    return res;
}
