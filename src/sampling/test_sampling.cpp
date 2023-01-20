#include "check_ddf.h"

#include "sampling.h"
#include "../GeometrySphereInBox.h"

#include <iostream>
#include <vector>

using namespace glm;
using namespace std;

const float EPS=1e-6;

bool eq(float a, float b){
    return abs(a-b) < EPS;
}

void test_ddf_basic(){
    UpperHalfDdf ud;
    assert(ud.max_value==1.0f);
    assert(!ud.isSingular());

    assert(ud.value(vec3())==1.0f);
    assert(ud.value(normalize(vec3(1,1,1)))==1.0f);
    assert(ud.value(normalize(vec3(1,1,-1)))==0.0f);

    CosineDdf cd;
    assert(eq(cd.max_value, 1.0f/M_PI));
    assert(!cd.isSingular());

    assert(eq(cd.value(vec3(0,0,1)),1.0f));
    assert(eq(cd.value(vec3(1,0,EPS/10.0f)),0.0f));
    assert(cd.value(normalize(vec3(1,1,-1)))==0.0f);

    MirrorDdf md;
    assert(isinf(md.max_value));
    assert(md.isSingular());
}

void test_sampling_ddfs(){

    cout << "UpperHalfDdf:" << endl;
    cout << (check_ddf(UpperHalfDdf()) ? "OK" : "FAIL") << endl;
    cout << endl;

    cout << "CosineDdf:" << endl;
    cout << (check_ddf(CosineDdf()) ? "OK" : "FAIL") << endl;
    cout << endl;

//    GeometrySphereInBox geo;

//    std::optional<surface_intersection> floor_inter = geo.traceRay(vec3(0.1, -1.1, -0.9), normalize(vec3(0.0f, 1.0f, -1.0f)));
//    assert(floor_inter.has_value());
//    cout << "floor:" << endl;
//    cout << (check_ddf(*floor_inter->sdf) ? "OK" : "FAIL") << endl;
//    cout << endl;

//    std::optional<surface_intersection> wall_inter = geo.traceRay(vec3(-0.9, -1.1, 0.1), normalize(vec3(-1.0f, 1.0f, 0.0f)));
//    assert(wall_inter.has_value());
//    cout << "wall:" << endl;
//    cout << (check_ddf(*wall_inter->sdf) ? "OK" : "FAIL") << endl;
//    cout << endl;

//    std::optional<surface_intersection> sphere_inter = geo.traceRay(vec3(0.1, -3.0, 0.05), vec3(0.0f, 1.0f, 0.0f));
//    assert(sphere_inter.has_value());
//    cout << "sphere:" << endl;
//    cout << (check_ddf(*sphere_inter->sdf) ? "OK" : "FAIL") << endl;
//    cout << endl;
}

int main(){
    test_ddf_basic();
    return 0;
}
