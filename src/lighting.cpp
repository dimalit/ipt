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

class LightToDistribution: public DdfImpl {
    // TODO Think about this
    friend class Superposition;
private:
    std::shared_ptr<const LightImpl> light;
    glm::vec3 origin;
public:
    LightToDistribution(std::shared_ptr<const LightImpl> light, glm::vec3 origin) {
        // TODO this is rough estimation, but for relatively small lights will work
        this->full_theoretical_weight = light->power / pow((light->position-origin).length(), 2);
        this->light = light;
        this->origin = origin;
    }
    virtual glm::vec3 trySample() const override;
    virtual float value( glm::vec3 direction ) const override;
};

glm::vec3 LightToDistribution::trySample() const {
    std::optional<light_intersection> inter = light->sample();
    if(!inter.has_value())            // if failed
        return glm::vec3();
    glm::vec3 dir = glm::normalize(inter->position-origin);
    float cosinus = dot(inter->normal, -dir);
    if(cosinus <= 0.0f)               // if facing back
        return glm::vec3();
    return randf() <= cosinus ? dir : vec3();
}

float LightToDistribution::value( glm::vec3 direction ) const {
    std::optional<light_intersection> inter = light->traceRay(origin, direction);
    if(!inter.has_value())
        return 0.0f;
    glm::vec3 dir = glm::normalize(inter->position-origin);
    float cosinus = dot(inter->normal, -dir);
    assert(cosinus >= 0.0f);
    return cosinus;
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
    float full_area = cross(x_axis, y_axis).length();
    area = type==TYPE_DIAMOND ? full_area : full_area/2.0f;
}
std::optional<light_intersection> AreaLight::sample() const {
    float u1 = randf();
    float u2 = randf() * (type==TYPE_TRIANLE ? 1.0f-u1 : 1.0f );
    vec3 pos = x_axis*u1 + y_axis*u2;

    light_intersection res;
    res.position = pos;
    res.normal = normalize(cross(x_axis, y_axis));

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
    float t = dot(n, this->position-origin) / n_dir;
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
    res.position = pos;
    res.normal = normalize(cross(x_axis, y_axis));

    return res;
}

optional<light_intersection> SphereLight::traceRay(glm::vec3 origin, glm::vec3 direction) const {
    float t = intersection_with_sphere(radius, origin, direction);
    if(t == std::numeric_limits<float>::infinity())
        return {};

    light_intersection res;
    res.position = origin + direction*t;
    res.normal = normalize(res.position - this->position);

    return res;
}

optional<light_intersection> SphereLight::sample() const {

    float u1 = randf()*2.0f - 1.0f;     // -1..+1
    float u2 = randf();
    float alpha = acos(u1);
    float phi = 2*M_PI*u2;
    float r = sin(alpha);
    vec3 pos(r*cos(phi), r*sin(phi), u1);

    light_intersection res;
    res.position = pos;
    res.normal = normalize(res.position - this->position);

    return res;
}
