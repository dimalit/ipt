#ifndef SAMPLING_DETAIL_H
#define SAMPLING_DETAIL_H

#include "sampling.h"

#include <vector>

class Superposition: public DistributionImpl {

private:
    std::shared_ptr<const DistributionImpl> source, dest;
    static float magic_compute_max_pdf(const DistributionImpl* source, const DistributionImpl* dest);

public:

    Superposition(std::shared_ptr<const Distribution> _source, std::shared_ptr<const Distribution> _dest);

    virtual float pdf( glm::vec3 x ) const override {
        return source->pdf(x) * dest->pdf(x);
    }

    virtual glm::vec3 trySample() const override;
};

// TODO Indicate somehow that trySample should always succeed!
struct Union: public Distribution {
    std::vector< std::shared_ptr<const Distribution> > components;
    // weight eqals to sum of weights
    virtual glm::vec3 trySample() const override;
};

#endif // SAMPLING_DETAIL_H
