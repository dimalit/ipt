#ifndef LIGHTING_H
#define LIGHTING_H

#include <ddf_detail.h>
#include <tracer_interfaces.h>

struct Light {
    virtual std::optional<light_intersection> traceRay(glm::vec3 origin, glm::vec3 direction) const = 0;
    virtual std::unique_ptr<Ddf> lightToPoint(glm::vec3 pos) const;
    float power;
    float area;
    glm::vec3 position;

    virtual light_intersection sample() const = 0;
};

class AreaLight: public Light {
public:
    enum type_t {TYPE_DIAMOND, TYPE_TRIANLE};
private:
    glm::vec3 x_axis, y_axis;
    glm::mat3 inverse_matrix;
    type_t type;

public:
    AreaLight(glm::vec3 origin, glm::vec3 x_axis, glm::vec3 y_axis, float power, type_t type=TYPE_DIAMOND);
    virtual std::optional<light_intersection> traceRay(glm::vec3 origin, glm::vec3 direction) const override;
    virtual light_intersection sample() const override;
};

class PointLight:public Light {
public:
    PointLight(glm::vec3 origin, float virtual_radius, float power=1.0f){
        this->position = origin;
        this->power = power;
        area = 0.0f;
    }

    virtual std::optional<light_intersection> traceRay(glm::vec3, glm::vec3) const override {
        return {};
    }

    virtual light_intersection sample() const override;
};

struct SphereLight: public Light {
    float radius;
    SphereLight(glm::vec3 origin, float radius, float power=1.0f){
        this->position = origin;
        this->power = power;
        this->radius = radius;
        area = 4.0 * M_PI * radius*radius;
    }
    virtual std::optional<light_intersection> traceRay(glm::vec3 origin, glm::vec3 direction) const override;
    virtual light_intersection sample() const override;
};

// for testing
struct InvertedSphereLight: public SphereLight {
    InvertedSphereLight(glm::vec3 origin, float radius, float power)
        :SphereLight(origin, radius, power){}
    virtual std::optional<light_intersection> traceRay(glm::vec3 origin, glm::vec3 direction) const override {
        std::optional<light_intersection> res = SphereLight::traceRay(origin, direction);
        if(res.has_value())
            res->normal = -res->normal;
        return res;
    }
    virtual light_intersection sample() const override {
        light_intersection res = SphereLight::sample();
        res.normal = -res.normal;
        return res;
    }
};

#endif // LIGHTING_H
