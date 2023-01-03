#ifndef LIGHTING_H
#define LIGHTING_H

#include "sampling.h"

#include "tracer_interfaces.h"

struct Light {
    virtual std::optional<light_intersection> traceRay(glm::vec3 origin, glm::vec3 direction) const = 0;
    virtual std::optional<light_intersection> trySample() const = 0;
    float weight;
    glm::vec3 origin;
};

class LightToDistribution: public DistributionImpl {
    // TODO Think about this
    friend class Superposition;
private:
    std::shared_ptr<const Light> light;
    glm::vec3 origin;
public:
    LightToDistribution(std::shared_ptr<const Light> light, glm::vec3 origin) {
        this->weight = light->weight;
        this->light = light;
        this->origin = origin;
    }
    virtual glm::vec3 trySample() const override {
        std::optional<light_intersection> inter = light->trySample();
        if(!inter.has_value())            // if failed
            return glm::vec3();
        glm::vec3 dir = glm::normalize(inter->position-origin);
        return dir;
    }
    virtual float pdf( glm::vec3 direction ) const {
        std::optional<light_intersection> inter = light->traceRay(origin, direction);
        if(!inter.has_value())
            return 0.0f;
        return inter->radiation / pow((light->origin-origin).length(), 2);
    }
};

class AreaLight: public Light {
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

class PointLight:public Light {
public:
    PointLight(glm::vec3 origin, float weight){
        this->origin = origin;
        this->weight = weight;
    }

    virtual std::optional<light_intersection> traceRay(glm::vec3, glm::vec3) const override {
        return {};
    }

    virtual std::optional<light_intersection> trySample() const override {
        light_intersection res;
        res.position = this->origin;
        res.radiation = weight;
        return res;
    }
};

struct SphereLight: public Light {
    float radius;
    SphereLight(glm::vec3 origin, float weight, float radius){
        this->origin = origin;
        this->weight = weight;
        this->radius = radius;
    }
    virtual std::optional<light_intersection> traceRay(glm::vec3 origin, glm::vec3 direction) const override;
    virtual std::optional<light_intersection> trySample() const override;
};

#endif // LIGHTING_H
