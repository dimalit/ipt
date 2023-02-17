#include "lighting.h"

#include <check_ddf.h>

#include <catch_amalgamated.hpp>

#include <iostream>

using namespace glm;
using namespace std;

const size_t n_samples = 100000;

float uniform_sampling(vec3 origin, Ddf* sdf, const Light& light){

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

float directed_sampling(vec3 origin, Ddf* sdf, const Light& light){

    shared_ptr<Ddf> light_ddf = light.lightToPoint(origin);
    //shared_ptr<Ddf> chain_ddf = chain(light_ddf, sdf);

    float sum = 0.0f;
    for(size_t i = 0; i < n_samples; ++i){
        vec3 dir = light_ddf->trySample();

        // add dark faces as 0's
        if(dir == vec3())
            continue;

        light_intersection inter = Lighting::last_sample;
        // HACK: dir==vec3() means take from Lighting::last_sample
        sum += inter.surface_power / light_ddf->value(vec3()) * sdf->value(dir);
    }

    return sum/n_samples;
}

template<class SDF, class S>
void test_all_lights(vec3 origin, vec3 direction, S f){
    shared_ptr<const Light> area_light = make_shared<AreaLight>(vec3(-0.5f,-0.5f,0), vec3(1,0,0), vec3(0,1,0), 1.0f);
    shared_ptr<const Light> sphere_light = make_shared<SphereLight>(vec3(), 0.5f);

    unique_ptr<Ddf> sdf = make_unique<RotateDdf>(unique_ptr<SDF>(), direction);

    float v1 = f(origin, sdf.get(), *area_light);
    origin *= 2;
    float v2 = f(origin, sdf.get(), *area_light);

    cout << v1 << " / " << v2 << " = " << v1/v2 << endl;

    origin /= 2;
    float v3 = f(origin, sdf.get(), *sphere_light);

//    cout << v1 << " / " << v3 << " = " << v1/v3 << endl;

    cout << v2 << " / " << v3 << " = " << v2/v3 << endl;

    origin *= 2;
    float v4 = f(origin, sdf.get(), *sphere_light);

    cout << v3 << " / " << v4 << " = " << v3/v4 << endl;
}

void test_light_ddfs(vec3 origin){
    shared_ptr<const Light> area_light = make_shared<AreaLight>(vec3(-0.5f,-0.5f,0), vec3(1,0,0), vec3(0,1,0), 1.0f);
    shared_ptr<const Light> sphere_light = make_shared<SphereLight>(vec3(), 0.5f);

    shared_ptr<Ddf> light_ddf;

    light_ddf = area_light->lightToPoint(origin);
    cout << "Area:" << endl;
    cout << (check_ddf(*light_ddf) ? "OK" : "FAIL") << endl;
    cout << endl;

    light_ddf = area_light->lightToPoint(origin*2.0f);
    cout << "Area*2:" << endl;
    cout << (check_ddf(*light_ddf) ? "OK" : "FAIL") << endl;
    cout << endl;

//    light_ddf = sphere_light->lightToPoint(origin);
//    cout << "Sphere:" << endl;
//    cout << (check_ddf(*light_ddf) ? "OK" : "FAIL") << endl;
//    cout << endl;

//    light_ddf = sphere_light->lightToPoint(origin*2.0f);
//    cout << "Sphere*2:" << endl;
//    cout << (check_ddf(*light_ddf) ? "OK" : "FAIL") << endl;
//    cout << endl;
}

int old_main(){

//    cout << "Up:" << endl;
//    test_light_ddfs(vec3(0,0,0.6f));
//    cout << "Side:" << endl;
//    test_light_ddfs(vec3(0.5f,0,0.5f));
//    return 0;

///

    cout << "Uniform up:" << endl;
    test_all_lights<CosineDdf>(vec3(0,0,1), vec3(0,0,-1), uniform_sampling);
    cout << "Directed up:" << endl;
    test_all_lights<CosineDdf>(vec3(0,0,1), vec3(0,0,-1), directed_sampling);

    cout << "Uniform side:" << endl;
    test_all_lights<CosineDdf>(vec3(0.5,0,1), vec3(-1,0,0), uniform_sampling);
    cout << "Directed side:" << endl;
    test_all_lights<CosineDdf>(vec3(0.5,0,1), vec3(-1,0,0), directed_sampling);

    cout << "Number trios after '=' should be approximately equal for uniform and directed" << endl;

    return 0;
}

using namespace Catch::Matchers;

TEST_CASE("AreaLight basic tests"){ 
    AreaLight diag(vec3(1,1,1), vec3(-1,-1,-1), vec3(0,-1,0), 4);

    const float eps = 1e-6;
    float sin_alpha = sqrt(2.0f/3.0f);
    float diag_length = length(vec3(1,1,1));
    REQUIRE_THAT(diag.area, WithinAbs(diag_length*sin_alpha, eps));

    vec3 origin = vec3(0,0,0.1f);
    vec3 diamond_hit_dir = vec3(1.1f, 0, 0);

    optional<light_intersection> inter = diag.traceRay(origin, diamond_hit_dir);
    REQUIRE(inter.has_value());
    REQUIRE_THAT(inter->surface_power, WithinAbs(4.0f/diag.area, eps));
}

