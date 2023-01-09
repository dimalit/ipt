#ifndef LIGHTING_H
#define LIGHTING_H

#include "sampling.h"
#include "tracer_interfaces.h"

struct Light {
    virtual std::optional<light_intersection> traceRay(glm::vec3 origin, glm::vec3 direction) const = 0;
    virtual std::shared_ptr<const Ddf> lightToPoint(glm::vec3 pos) const;
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
    type_t type;

public:
    AreaLight(glm::vec3 origin, glm::vec3 x_axis, glm::vec3 y_axis, float power, type_t type=TYPE_DIAMOND);
    virtual std::optional<light_intersection> traceRay(glm::vec3 origin, glm::vec3 direction) const override;
    virtual light_intersection sample() const override;
};

class PointLight:public Light {
public:
    PointLight(glm::vec3 origin, float power){
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
    SphereLight(glm::vec3 origin, float power, float radius){
        this->position = origin;
        this->power = power;
        this->radius = radius;
        area = 4.0 * M_PI * radius;
    }
    virtual std::optional<light_intersection> traceRay(glm::vec3 origin, glm::vec3 direction) const override;
    virtual light_intersection sample() const override;
};

#endif // LIGHTING_H
