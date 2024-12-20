#ifndef DDF_H
#define DDF_H

#include <glm/vec3.hpp>

#include <boost/pool/poolfwd.hpp>

#include <memory>
#include <iostream>
#include <atomic>

struct Ddf {

    // Sample a signle vec3 value. Always succeedes
    virtual glm::vec3 sample() const = 0;

    // Get value in direction of arg. For singulars returns 0
    virtual float value( glm::vec3 arg ) const = 0;

    //static std::atomic_size_t object_counter;

    Ddf(){
        // ++object_counter;
        // static thread_local size_t max = 0;
        // if(object_counter>max){
        //     max=object_counter;
        //     std::cout << "COUNT " << max << std::endl;
        // }
        //std::cout << "COUNT " << object_counter << " / " << max << std::endl;
    }

    virtual ~Ddf(){
        //--object_counter;
        //std::cout << "COUNT " << object_counter << std::endl;
    }

    void* operator new(size_t size);
    void operator delete(void* ptr);
};

extern std::unique_ptr<Ddf> unite(std::unique_ptr<Ddf> a = nullptr, float ka=1.0f, std::unique_ptr<Ddf> b = nullptr, float kb=1.0f);

#endif // DDF_H
