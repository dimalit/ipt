#ifndef CHECK_DDF_H
#define CHECK_DDF_H

#include "ddf.h"

#include "randf.h"

#include <glm/geometric.hpp>

extern bool check_ddf(const Ddf& ddf, bool strict_integral=true, size_t size_alpha=20, size_t size_phi=20);

class SquareDdfForTest: public Ddf{
private:
    float side;
public:
    SquareDdfForTest(float _side = 1.0f): side(_side){}
    virtual glm::vec3 trySample() const override {
        float u1 = randf()-0.5f;
        float u2 = randf()-0.5f;
        glm::vec3 pos = glm::vec3(0,0,1) + glm::vec3(u1*side,0,0) + glm::vec3(0,u2*side,0);
        glm::vec3 dir = glm::normalize(pos);
        return dir;
    }
    // const here! it made mistake already (arg/=z)!!
    virtual float value( glm::vec3 arg ) const override {
        if(arg.z <= 0.0f)
            return 0.0f;

        // check intersection
        glm::vec3 inter = arg / arg.z;

        if(std::abs(inter.x) > side/2 || std::abs(inter.y) > side/2)
            return 0.0f;

        float area_density = 1.0/side/side;
        float cosinus=arg.z;
        float angular_density = area_density*glm::dot(inter, inter)/cosinus;
        return angular_density;
    }
};

#endif // CHECK_DDF_H
