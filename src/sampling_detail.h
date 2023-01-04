#ifndef SAMPLING_DETAIL_H
#define SAMPLING_DETAIL_H

#include "sampling.h"

#include <vector>

class SuperpositionDdf: public DdfImpl {

private:
    std::shared_ptr<const DdfImpl> source, dest;
    static float magic_compute_max_pdf(const DdfImpl* source, const DdfImpl* dest);

public:

    SuperpositionDdf(std::shared_ptr<const Ddf> _source, std::shared_ptr<const Ddf> _dest);

    virtual float value( glm::vec3 x ) const override {
        return source->value(x) * dest->value(x);
    }

    virtual glm::vec3 trySample() const override;
};

// TODO Indicate somehow that trySample should always succeed!
struct UnionDdf: public Ddf {
    std::vector< std::shared_ptr<const Ddf> > components;
    // weight eqals to sum of weights
    virtual glm::vec3 trySample() const override;
};

#endif // SAMPLING_DETAIL_H
