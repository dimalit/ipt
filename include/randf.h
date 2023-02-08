#ifndef RANDF_H
#define RANDF_H

#include <random>

static std::random_device rdev;
static std::mt19937 gen = std::mt19937(rdev());

static struct {
    std::uniform_real_distribution<> dist = std::uniform_real_distribution<>(0.0f, 1.0f);

    float operator()(std::mt19937& _gen = gen) {
        float res = dist(_gen);
        while(res==1.0f)
            res = dist(_gen);
        return res;
    }
} randf;

#endif // RANDF_H
