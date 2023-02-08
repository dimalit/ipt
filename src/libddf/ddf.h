#ifndef DDF_H
#define DDF_H

#include "randf.h"

#include <glm/vec3.hpp>

#include <boost/pool/poolfwd.hpp>

#include <memory>
#include <iostream>
#include <map>

struct Ddf {

    // may return 0 if sampling failed
    // this is used to implicitly lower distribution's weight
    virtual glm::vec3 trySample(std::mt19937& _gen = gen) const = 0;

    void seed();

    // will return NaN if singular
    virtual float value( glm::vec3 arg ) const = 0;

    // used in sampling
    // is infinity if singular
    // must be initialized in subclasses
    float max_value = std::numeric_limits<float>::signaling_NaN();

    bool isSingular() const {
        return max_value == std::numeric_limits<float>::infinity();
    }

    static size_t object_counter;

    Ddf(){
        ++object_counter;
        static size_t max = 0;
        if(object_counter>max){
            max=object_counter;
            std::cout << "COUNT " << max << std::endl;
        }
        //std::cout << "COUNT " << object_counter << " / " << max << std::endl;
    }

    virtual ~Ddf(){
        --object_counter;
        //std::cout << "COUNT " << object_counter << std::endl;
    }

    void* operator new(size_t size);
    void operator delete(void* ptr);
};

extern std::unique_ptr<Ddf> unite(std::unique_ptr<Ddf> a = nullptr, float ka=1.0f, std::unique_ptr<Ddf> b = nullptr, float kb=1.0f);
extern std::unique_ptr<Ddf> chain(std::unique_ptr<Ddf> source, std::unique_ptr<Ddf> dest);

#endif // DDF_H
