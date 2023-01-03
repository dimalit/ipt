#ifndef SAMPLING_H
#define SAMPLING_H

#include <memory>

#include <glm/vec3.hpp>
#include <glm/mat3x3.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "sampling_fwd.h"

struct Distribution {

    // may return 0 if sampling failed
    // this is used to balance parts of union distribution
    virtual glm::vec3 trySample() const = 0;

    // used in parts of union distribution
    float weight = 1.0f;

};

struct DistributionImpl: public Distribution {

    // will return NaN if singular
    virtual float pdf( glm::vec3 arg ) const = 0;

    // used in sampling
    // is infinity if singular
    float max_pdf;

    bool pdf_integrates_to_one = true;

    bool isSingular() const {
        return max_pdf == std::numeric_limits<float>::infinity();
    }
};

// TODO hide t from interface?
struct Transform: public Distribution {
    std::shared_ptr<const Distribution> origin;
    glm::mat3 transformation;
    virtual glm::vec3 trySample() const override {
        glm::vec3 x = origin->trySample();
        return transformation * x;
    }
    // TODO Need separate implementations for Continuous and Singular
};

// TODO bad idea to inherit implementation!
struct Rotate: public Transform {
    Rotate(glm::vec3 to){
        glm::vec3 z = glm::vec3(0.0f, 0.0f, 1.0f);
        glm::vec3 axis = cross(z, to);
        float angle = dot(z,to);
        transformation = rotate(glm::identity<glm::mat4>(), angle, axis);
    }
};

struct UpperHalf: public DistributionImpl {
    UpperHalf();
    virtual glm::vec3 trySample() const override;
    virtual float pdf( glm::vec3 arg ) const override;
};

class Cosine: public DistributionImpl {
public:
    Cosine(float w);
    virtual glm::vec3 trySample() const override;
    virtual float pdf( glm::vec3 arg ) const override;
};

class Mirrorly: public DistributionImpl {
public:
    Mirrorly(float w){
        max_pdf = std::numeric_limits<float>::infinity();
        weight = w;
    }
    virtual glm::vec3 trySample() const override {
        return glm::vec3(0.0f, 0.0f, 1.0f);
    }
    virtual float pdf( glm::vec3 arg ) const override {
        return std::numeric_limits<float>::quiet_NaN();
    }
};

extern std::shared_ptr<Distribution> unite(std::shared_ptr<const Distribution> a, std::shared_ptr<const Distribution> b);
extern std::shared_ptr<const Distribution> apply(std::shared_ptr<const Distribution> source, std::shared_ptr<const Distribution> dest);

#endif // SAMPLING_H
