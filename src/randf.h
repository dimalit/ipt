#ifndef RANDF_H
#define RANDF_H

#include <random>

struct randf_t {
    std::random_device rdev;
    std::mt19937 gen = std::mt19937(rdev());

    std::uniform_real_distribution<> dist = std::uniform_real_distribution<>(0.0f, 1.0f);

    float operator()() {
        float res = dist(gen);
        while(res==1.0f)
            res = dist(gen);
        return res;
    }
};

extern randf_t randf;

#endif // RANDF_H
