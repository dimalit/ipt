#ifndef LIGHTING_H
#define LIGHTING_H

#include "sampling.h"

#include "tracer_interfaces.h"

struct Light {
    virtual std::optional<light_intersection> traceRay(glm::vec3 origin, glm::vec3 direction) const = 0;
    virtual std::shared_ptr<const Distribution> lightToPoint(glm::vec3 pos) const;
    float weight;
    glm::vec3 position;
};

struct LightImpl: public Light {
    virtual std::optional<light_intersection> trySample() const = 0;
};

class AreaLight: public LightImpl {
    // TODO Think about this
    friend class Superposition;
public:
    enum type_t {TYPE_DIAMOND, TYPE_TRIANLE};
private:
    glm::vec3 x_axis, y_axis;
    type_t type;

    float area() const {
        float full_area = cross(x_axis, y_axis).length();
        return type==TYPE_DIAMOND ? full_area : full_area/2.0f;
    }
public:
    AreaLight(glm::vec3 origin, glm::vec3 x_axis, glm::vec3 y_axis, float w, type_t type=TYPE_DIAMOND);
    virtual std::optional<light_intersection> traceRay(glm::vec3 origin, glm::vec3 direction) const override;
    virtual std::optional<light_intersection> trySample() const override;
};

class PointLight:public LightImpl {
public:
    PointLight(glm::vec3 origin, float weight){
        this->position = origin;
        this->weight = weight;
    }

    virtual std::optional<light_intersection> traceRay(glm::vec3, glm::vec3) const override {
        return {};
    }

    virtual std::optional<light_intersection> trySample() const override {
        light_intersection res;
        res.position = this->position;
        return res;
    }
};

struct SphereLight: public LightImpl {
    float radius;
    SphereLight(glm::vec3 origin, float weight, float radius){
        this->position = origin;
        this->weight = weight;
        this->radius = radius;
    }
    virtual std::optional<light_intersection> traceRay(glm::vec3 origin, glm::vec3 direction) const override;
    virtual std::optional<light_intersection> trySample() const override;
};

#endif // LIGHTING_H
