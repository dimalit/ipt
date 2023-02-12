#include "check_ddf.h"

#include "ddf_detail.h"

#include <catch_amalgamated.hpp>

#include <iostream>
#include <vector>

using namespace glm;
using namespace std;

const float EPS=1e-6;

bool eq(float a, float b){
    return abs(a-b) < EPS;
}

TEST_CASE("Basic checks for DDFs"){
    SphericalDdf sd;
    REQUIRE(eq(sd.max_value, 0.25f/M_PI));
    REQUIRE(!sd.isSingular());
    REQUIRE(eq(sd.value(normalize(vec3(1,1,1))),0.25f/M_PI));
    REQUIRE(eq(sd.value(normalize(vec3(1,1,-1))), 0.25f/M_PI));

    UpperHalfDdf ud;
    REQUIRE(eq(ud.max_value,0.5f/M_PI));
    REQUIRE(!ud.isSingular());

    REQUIRE(eq(ud.value(vec3(0,0,1)),0.5f/M_PI));
    REQUIRE(eq(ud.value(normalize(vec3(1,1,1))),0.5f/M_PI));
    REQUIRE(ud.value(normalize(vec3(1,1,-1)))==0.0f);

    CosineDdf cd;
    REQUIRE(eq(cd.max_value, 1.0f/M_PI));
    REQUIRE(!cd.isSingular());

    REQUIRE(eq(cd.value(vec3(0,0,1)),M_1_PI));
    REQUIRE(eq(cd.value(vec3(1,0,EPS/10.0f)),0.0f));
    REQUIRE(cd.value(normalize(vec3(1,1,-1)))==0.0f);

    MirrorDdf md;
    REQUIRE(isinf(md.max_value));
    REQUIRE(md.isSingular());
}

TEST_CASE("Chi^2 for DDFs"){

    CHECK(check_ddf(SphericalDdf()));
    CHECK(check_ddf(UpperHalfDdf()));
    CHECK(check_ddf(CosineDdf()));

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

TEST_CASE("Chi^2 for superpositions"){

    unique_ptr<Ddf> sup_self = chain(make_unique<UpperHalfDdf>(), make_unique<UpperHalfDdf>());
    CHECK(check_ddf(*sup_self));

    unique_ptr<Ddf> cosine = make_unique<CosineDdf>();

    SECTION("Right half-cosine"){
        unique_ptr<Ddf> right_half = make_unique<RotateDdf>(make_unique<UpperHalfDdf>(), vec3(1,0,0));
        unique_ptr<Ddf> half_cosine = chain(move(right_half), move(cosine));
        CHECK(check_ddf(*half_cosine, false));
    }

    unique_ptr<Ddf> square = make_unique<SquareDdfForTest>(1.0f);

    SECTION("Plain 1x1 square"){
        CHECK(check_ddf(*square, true, 40, 20));
    }

    SECTION("Square over cosine"){
        unique_ptr<Ddf> square_over_cosine = chain(move(square), move(cosine));
        CHECK(check_ddf(*square_over_cosine, false, 40, 20));
    }

    SECTION("Tilted square over cosine"){
        unique_ptr<Ddf> tilted_square_over_cosine = chain(make_unique<RotateDdf>(move(square), normalize(vec3(1,1,1))), move(cosine));
        CHECK(check_ddf(*tilted_square_over_cosine, false, 80, 40));
    }

    // Very important case: square over inverse itself!
    // TODO do same copy trick in other tests
    SECTION("Big square over inverse itself"){
        SquareDdfForTest big_square(10.0f);
        float min_value = big_square.value(vec3(0,0,1));
        unique_ptr<Ddf> inverse = make_unique<InvertDdf>(make_unique<SquareDdfForTest>(big_square), min_value);
        unique_ptr<Ddf> square_over_inverse = chain(make_unique<SquareDdfForTest>(big_square), move(inverse));
        CHECK(check_ddf(*square_over_inverse, false));
    }
}

TEST_CASE("Chi^2 for union left+right square"){
    unique_ptr<Ddf> right = make_unique<RotateDdf>(make_unique<SquareDdfForTest>(), normalize(vec3(1,0,1)));
    unique_ptr<Ddf> left  = make_unique<RotateDdf>(make_unique<SquareDdfForTest>(), normalize(vec3(-1,0,1)));

    SECTION("1:1"){
        unique_ptr<Ddf> both  = unite(move(left), 1.0f, move(right), 1.0f);
        CHECK(check_ddf(*both, true, 40, 40));
    }
    SECTION("1:2.5"){
        unique_ptr<Ddf> unequal  = unite(move(left), 1.0f, move(right), 2.5f);
        CHECK(check_ddf(*unequal, true, 40, 40));
    }
    SECTION("Right 1/2 on horizon"){
        unique_ptr<Ddf> right_on_horizon = chain(make_unique<RotateDdf>(make_unique<SquareDdfForTest>(), vec3(1,0,0)), make_unique<UpperHalfDdf>());
        SECTION("Just right"){
            CHECK(check_ddf(*right_on_horizon, false, 80, 80));
        }
        SECTION("Union left 1 + right 1/2 on horizon 2.5"){
            // TODO check here that integral equal 2.25/3.5=0.64 (it is)
            unique_ptr<Ddf> unequal_with_horizon = unite(move(left), 1.0f, move(right_on_horizon), 2.5f);
            CHECK(check_ddf(*unequal_with_horizon, false, 40, 40));
        }
    }
}
