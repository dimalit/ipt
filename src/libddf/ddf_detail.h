#ifndef SAMPLING_H
#define SAMPLING_H

#include "ddf.h"

#include <glm/vec3.hpp>
#include <glm/mat3x3.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <memory>

namespace detail {

// TODO hide t from interface?
struct TransformDdf: public Ddf {
    std::unique_ptr<Ddf> origin;
    glm::mat3 transformation;
    TransformDdf(std::unique_ptr<Ddf> _origin, glm::mat3 _transformation)
        :origin(std::move(_origin))
    {
        assert(this->origin);
        this->transformation = transformation;
        // XXX best-guess
        this->max_value = dynamic_cast<Ddf*>(origin.get())->max_value;
    }
    virtual glm::vec3 trySample(std::mt19937& _gen = gen) const override {
        glm::vec3 x = origin->trySample(_gen);
        return transformation * x;
    }

    virtual float value( glm::vec3 arg ) const override {
        return origin->value(inverse(transformation)*arg);
    }

    // TODO Need separate implementations for Continuous and Singular
};

}// namespace

struct SphericalDdf: public Ddf {
    SphericalDdf();
    virtual glm::vec3 trySample(std::mt19937& _gen = gen) const override;
    virtual float value( glm::vec3 arg ) const override;
};

struct UpperHalfDdf: public Ddf {
    UpperHalfDdf();
    virtual glm::vec3 trySample(std::mt19937& _gen = gen) const override;
    virtual float value( glm::vec3 arg ) const override;
};

class CosineDdf: public Ddf {
public:
    CosineDdf();
    virtual glm::vec3 trySample(std::mt19937& _gen = gen) const override;
    virtual float value( glm::vec3 arg ) const override;
};

class MirrorDdf: public Ddf {
public:
    MirrorDdf(){
        max_value = std::numeric_limits<float>::infinity();
    }
    virtual glm::vec3 trySample(std::mt19937& _gen = gen) const override {
        return glm::vec3(0.0f, 0.0f, 1.0f);
    }
    virtual float value( glm::vec3 arg ) const override {
        return std::numeric_limits<float>::quiet_NaN();
    }
};

// TODO bad idea to inherit implementation!
struct RotateDdf: public detail::TransformDdf {
    RotateDdf(std::unique_ptr<Ddf> origin, glm::vec3 to)
        :TransformDdf(move(origin), glm::mat3()){
        glm::vec3 z = glm::vec3(0.0f, 0.0f, 1.0f);
        glm::vec3 axis = cross(z, to);
        // HACK corner case to=z
        if(length(axis)<1e-6)
            axis = glm::vec3(1.0f, 0.0f, 0.0f);
        float cosinus = dot(z,to);
        transformation = rotate(glm::identity<glm::mat4>(), (float)acos(cosinus), axis);
    }
};

// TODO Probably this should exist as already chain of 2
struct InvertDdf: public Ddf {
    std::unique_ptr<Ddf> origin;
    InvertDdf(std::unique_ptr<Ddf> _origin, float min_value)
        :origin(std::move(_origin)){
        assert(this->origin);
        this->max_value = 1.0f/min_value;
    }
    virtual glm::vec3 trySample(std::mt19937& _gen = gen) const override {
        // TODO Do it better?
        assert(false && "not implemented");
        return glm::vec3();
    }

    virtual float value( glm::vec3 arg ) const override {
        float v = origin->value(arg);
        return v==0.0f ? 0.0f : 1.0f/v;
    }
};

#endif // SAMPLING_H
