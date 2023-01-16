#include "lighting.h"

#include <iostream>

using namespace glm;
using namespace std;

const size_t n_samples = 100000;

float uniform_sampling(vec3 origin, shared_ptr<const Ddf> sdf, const Light& light){

    float sum = 0.0f;
    for(size_t i = 0; i < n_samples; ++i){
        vec3 dir = sdf->trySample();
        assert(dir != vec3());
        optional<light_intersection> inter = light.traceRay(origin, dir);
        if(!inter.has_value())
            continue;
        sum += inter->surface_power;
    }

    return sum/n_samples;
}

float directed_sampling(vec3 origin, shared_ptr<const Ddf> sdf, const Light& light){

    shared_ptr<const Ddf> light_ddf = light.lightToPoint(origin);
    shared_ptr<const Ddf> chain_ddf = chain(light_ddf, sdf);

    const ::detail::DdfImpl* chain_impl = dynamic_cast<const ::detail::DdfImpl*>(chain_ddf.get());

    float sum = 0.0f;
    for(size_t i = 0; i < n_samples; ++i){
        vec3 dir = chain_ddf->trySample();
        //assert(dir != vec3());
        if(dir == vec3())
            continue;
        light_intersection inter = Lighting::last_sample;
        sum += inter.surface_power / chain_impl->value(dir);
    }

    return sum/n_samples;
}

template<class SDF>
void test_two_samplings(vec3 origin, vec3 direction, const Light& light){
    shared_ptr<const Ddf> sdf = make_shared<RotateDdf>(make_shared<SDF>(), direction);

    float v1 = uniform_sampling(origin, sdf, light);
    origin *= 2;
    float v2 = uniform_sampling(origin, sdf, light);

    cout << "Uniform: " << v1 << " / " << v2 << " = " << v1/v2 << endl;

    origin /= 2;
    v1 = directed_sampling(origin, sdf, light);
    origin *= 2;
    v2 = directed_sampling(origin, sdf, light);

    cout << "Directed: " << v1 << " / " << v2 << " = " << v1/v2 << endl;

}

int main(){

    // down-facing SDF
    //shared_ptr<const Ddf> sdf = make_shared<RotateDdf>(make_shared<UpperHalfDdf>(), vec3(0,0,-1));
    //shared_ptr<const Ddf> sdf = make_shared<RotateDdf>(make_shared<CosineDdf>(), vec3(0,0,-1));

    // 1x1 in XY plane
    shared_ptr<const Light> light = make_shared<AreaLight>(vec3(-0.5f,-0.5f,0), vec3(1,0,0), vec3(0,1,0), 1.0f);
    cout << "Area up:" << endl;
    test_two_samplings<CosineDdf>(vec3(0,0,1), vec3(0,0,-1), *light);
    cout << "Area side:" << endl;
    test_two_samplings<CosineDdf>(vec3(0.5,0,1), vec3(-1,0,0), *light);

    light = make_shared<SphereLight>(vec3(), 0.5f);
    cout << "Sphere up:" << endl;
    test_two_samplings<CosineDdf>(vec3(0,0,1), vec3(0,0,-1), *light);
    cout << "Sphere side:" << endl;
    test_two_samplings<CosineDdf>(vec3(0.5,0,1), vec3(-1,0,0), *light);

    return 0;
}
