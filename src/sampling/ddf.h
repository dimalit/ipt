#ifndef DDF_H
#define DDF_H

#include <glm/vec3.hpp>

#include <memory>

struct Ddf {

    // may return 0 if sampling failed
    // this is used to implicitly lower distribution's weight
    virtual glm::vec3 trySample() const = 0;

    float full_theoretical_weight = 1.0f;

    // will return NaN if singular
    virtual float value( glm::vec3 arg ) const = 0;

    // used in sampling
    // is infinity if singular
    // must be initialized in subclasses
    float max_value = std::numeric_limits<float>::signaling_NaN();

    bool isSingular() const {
        return max_value == std::numeric_limits<float>::infinity();
    }

    virtual std::shared_ptr<const Ddf> multiply(float coef) const = 0;
};

extern std::shared_ptr<Ddf> unite(std::shared_ptr<const Ddf> a = nullptr, std::shared_ptr<const Ddf> b = nullptr);
extern std::shared_ptr<const Ddf> chain(std::shared_ptr<const Ddf> source, std::shared_ptr<const Ddf> dest);

#endif // DDF_H
