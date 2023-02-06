#include "check_ddf.h"

#include "ddf_detail.h"

#include <iostream>
#include <vector>

using namespace glm;
using namespace std;

const float EPS=1e-6;

bool eq(float a, float b){
    return abs(a-b) < EPS;
}

void test_ddf_basic(){
    SphericalDdf sd;
    assert(eq(sd.max_value, 0.25f/M_PI));
    assert(!sd.isSingular());
    assert(eq(sd.value(normalize(vec3(1,1,1))),0.25f/M_PI));
    assert(eq(sd.value(normalize(vec3(1,1,-1))), 0.25f/M_PI));

    UpperHalfDdf ud;
    assert(eq(ud.max_value,0.5f/M_PI));
    assert(!ud.isSingular());

    assert(eq(ud.value(vec3(0,0,1)),0.5f/M_PI));
    assert(eq(ud.value(normalize(vec3(1,1,1))),0.5f/M_PI));
    assert(ud.value(normalize(vec3(1,1,-1)))==0.0f);

    CosineDdf cd;
    assert(eq(cd.max_value, 1.0f/M_PI));
    assert(!cd.isSingular());

    assert(eq(cd.value(vec3(0,0,1)),M_1_PI));
    assert(eq(cd.value(vec3(1,0,EPS/10.0f)),0.0f));
    assert(cd.value(normalize(vec3(1,1,-1)))==0.0f);

    MirrorDdf md;
    assert(isinf(md.max_value));
    assert(md.isSingular());
}

void test_sampling_ddfs(){

    cout << "SphericalDdf:" << endl;
    cout << (check_ddf(SphericalDdf()) ? "OK" : "FAIL") << endl;
    cout << endl;

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

void test_superposition(){

    unique_ptr<Ddf> sup_self = chain(make_unique<UpperHalfDdf>(), make_unique<UpperHalfDdf>());

    cout << "UpperHalfDdf * 2:" << endl;
    cout << (check_ddf(*sup_self) ? "OK" : "FAIL") << endl;
    cout << endl;

    unique_ptr<Ddf> right_half = make_unique<RotateDdf>(make_unique<UpperHalfDdf>(), vec3(1,0,0));
    unique_ptr<Ddf> cosine = make_unique<CosineDdf>();
    unique_ptr<Ddf> half_cosine = chain(move(right_half), move(cosine));

    cout << "Right half-cosine:" << endl;
    cout << (check_ddf(*half_cosine, false) ? "OK" : "FAIL") << endl;
    cout << endl;

    unique_ptr<Ddf> square = make_unique<SquareDdfForTest>(1.0f);
    cout << "Plain 1x1 square:" << endl;
    cout << (check_ddf(*square, true, 40, 20) ? "OK" : "FAIL") << endl;
    cout << endl;

    square = make_unique<SquareDdfForTest>(1.0f);
    cosine = make_unique<CosineDdf>();
    unique_ptr<Ddf> square_over_cosine = chain(move(square), move(cosine));
    square = make_unique<SquareDdfForTest>(1.0f);
    cosine = make_unique<CosineDdf>();
    unique_ptr<Ddf> tilted_square_over_cosine = chain(make_unique<RotateDdf>(move(square), normalize(vec3(1,1,1))), move(cosine));

    cout << "Square over cosine:" << endl;
    cout << (check_ddf(*square_over_cosine, false, 40, 20) ? "OK" : "FAIL") << endl;
    cout << endl;

    cout << "Tilted square over cosine:" << endl;
    cout << (check_ddf(*tilted_square_over_cosine, false, 80, 40) ? "OK" : "FAIL") << endl;
    cout << endl;

    // Very important case: square over inverse itself!
    // TODO do same copy trick in other tests
    SquareDdfForTest big_square(10.0f);
    float min_value = big_square.value(vec3(0,0,1));
    unique_ptr<Ddf> inverse = make_unique<InvertDdf>(make_unique<SquareDdfForTest>(big_square), min_value);
    unique_ptr<Ddf> square_over_inverse = chain(make_unique<SquareDdfForTest>(big_square), move(inverse));

    cout << "Big square over inverse itself:" << endl;
    cout << (check_ddf(*square_over_inverse, false) ? "OK" : "FAIL") << endl;
    cout << endl;
}

void test_union(){
    unique_ptr<Ddf> right = make_unique<RotateDdf>(make_unique<SquareDdfForTest>(), normalize(vec3(1,0,1)));
    unique_ptr<Ddf> left  = make_unique<RotateDdf>(make_unique<SquareDdfForTest>(), normalize(vec3(-1,0,1)));

    unique_ptr<Ddf> both  = unite(move(left), 1.0f, move(right), 1.0f);

    cout << "Union left+right square:" << endl;
    cout << (check_ddf(*both, true, 40, 40) ? "OK" : "FAIL") << endl;
    cout << endl;

    right = make_unique<RotateDdf>(make_unique<SquareDdfForTest>(), normalize(vec3(1,0,1)));
    left  = make_unique<RotateDdf>(make_unique<SquareDdfForTest>(), normalize(vec3(-1,0,1)));
    unique_ptr<Ddf> unequal  = unite(move(left), 1.0f, move(right), 2.5f);

    cout << "Union left 1 + right 2.5:" << endl;
    cout << (check_ddf(*unequal, true, 40, 40) ? "OK" : "FAIL") << endl;
    cout << endl;

    unique_ptr<Ddf> right_on_horizon = chain(make_unique<RotateDdf>(make_unique<SquareDdfForTest>(), vec3(1,0,0)), make_unique<UpperHalfDdf>());
    cout << "Right 1/2 on horizon:" << endl;
    cout << (check_ddf(*right_on_horizon, false, 80, 80) ? "OK" : "FAIL") << endl;
    cout << endl;

    // TODO check here that integral equal 2.25/3.5=0.64 (it is)
    left  = make_unique<RotateDdf>(make_unique<SquareDdfForTest>(), normalize(vec3(-1,0,1)));
    unique_ptr<Ddf> unequal_with_horizon = unite(move(left), 1.0f, move(right_on_horizon), 2.5f);
    cout << "Union left 1 + right 1/2 on horizon 2.5:" << endl;
    cout << (check_ddf(*unequal_with_horizon, false, 40, 40) ? "OK" : "FAIL") << endl;
    cout << endl;
}

int main(){
    test_ddf_basic();
    test_sampling_ddfs();
    test_superposition();

    test_union();
    return 0;
}
