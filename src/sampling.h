#ifndef SAMPLING_H
#define SAMPLING_H

#include <memory>

#include <glm/vec3.hpp>
#include <glm/mat3x3.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "sampling_fwd.h"

struct Ddf {

    // may return 0 if sampling failed
    // this is used to balance parts of union distribution
    virtual glm::vec3 trySample() const = 0;

    float full_theoretical_weight = 1.0f;

};

struct DdfImpl: public Ddf {

    // will return NaN if singular
    virtual float value( glm::vec3 arg ) const = 0;

    // used in sampling
    // is infinity if singular
    float max_value;

    bool isSingular() const {
        return max_value == std::numeric_limits<float>::infinity();
    }
};

// TODO hide t from interface?
struct TransformDdf: public DdfImpl {
    std::shared_ptr<const DdfImpl> origin;
    glm::mat3 transformation;
    TransformDdf(std::shared_ptr<const Ddf> origin, glm::mat3 transformation){
        this->origin = std::dynamic_pointer_cast<const DdfImpl>(origin);
        assert(this->origin);
        this->transformation = transformation;
        // XXX best-guess
        this->max_value = dynamic_cast<const DdfImpl*>(origin.get())->max_value;
    }
    virtual glm::vec3 trySample() const override {
        glm::vec3 x = origin->trySample();
        return transformation * x;
    }

    virtual float value( glm::vec3 arg ) const {
        return origin->value(inverse(transformation)*arg);
    }

    // TODO Need separate implementations for Continuous and Singular
};

// TODO bad idea to inherit implementation!
struct RotateDdf: public TransformDdf {
    RotateDdf(std::shared_ptr<const Ddf> origin, glm::vec3 to)
        :TransformDdf(origin, glm::mat3()){
        glm::vec3 z = glm::vec3(0.0f, 0.0f, 1.0f);
        glm::vec3 axis = cross(z, to);
        float angle = dot(z,to);
        transformation = rotate(glm::identity<glm::mat4>(), angle, axis);
    }
};

struct UpperHalfDdf: public DdfImpl {
    UpperHalfDdf();
    virtual glm::vec3 trySample() const override;
    virtual float value( glm::vec3 arg ) const override;
};

class CosineDdf: public DdfImpl {
public:
    CosineDdf(float w);
    virtual glm::vec3 trySample() const override;
    virtual float value( glm::vec3 arg ) const override;
};

class MirrorDdf: public DdfImpl {
public:
    MirrorDdf(float w){
        max_value = std::numeric_limits<float>::infinity();
        full_theoretical_weight = w;
    }
    virtual glm::vec3 trySample() const override {
        return glm::vec3(0.0f, 0.0f, 1.0f);
    }
    virtual float value( glm::vec3 arg ) const override {
        return std::numeric_limits<float>::quiet_NaN();
    }
};

extern std::shared_ptr<Ddf> unite(std::shared_ptr<const Ddf> a, std::shared_ptr<const Ddf> b);
extern std::shared_ptr<const Ddf> apply(std::shared_ptr<const Ddf> source, std::shared_ptr<const Ddf> dest);

#endif // SAMPLING_H
