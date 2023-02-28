#include "check_ddf.h"

#include "ddf_detail.h"

#include <catch_amalgamated.hpp>

#include <CImg.h>

#include <iostream>
#include <vector>

using namespace glm;
using namespace std;
using namespace cimg_library;

const float EPS=1e-6;

bool eq(float a, float b){
    return abs(a-b) < EPS;
}

class FigFixture {
private:
    CImg<float> vertices;
    CImgList<float> primitives;
    CImg<float> opacities;
protected:

    FigFixture(){
        axes();
    }

    void display(){

        size_t n = primitives.size();
        CImg<unsigned char>(640,640).display_object3d("fig 1", vertices, primitives, CImgList<unsigned char>(), opacities, true, 1, 3);//, colors, opacities);
    }

    mat3 camera_mat(){
        mat4 mat = identity<mat4>();
        // every rotate() seems to add new matrix on the right!
        // so we do rotations in the inverse order
        mat = rotate(mat, M_PI_2f32*1.25f, vec3(1.0f, 0.0f, 0.0f)); // x-axis, 90+ deg
        mat = rotate(mat, -M_PI_4f32*0.5f, vec3(0.0f, 0.0f, 1.0f)); // z-axis, -45/2 deg
        //mat = rotate(mat, -M_PIf32/7.0f, vec3(0.0f, 0.0f, 1.0f)); // correct tilt around z
        return mat;
    }

    void vector(const vec3& start, const vec3& dir, float opacity = 1.0f){

        mat3 mat = camera_mat();

        vec3 r_start = mat*start*1000.0f;
        vec3 r_dir = mat*dir*1000.0f;

        CImg<float> vertices(2,3);
        vertices(0,0)=r_start[0];
        vertices(0,1)=r_start[1];
        vertices(0,2)=r_start[2];

        vertices(1,0)=r_start[0]+r_dir[0];
        vertices(1,1)=r_start[1]+r_dir[1];
        vertices(1,2)=r_start[2]+r_dir[2];

        CImgList<float> primitives(1,1,2,1,1,
                               0, 1);
        CImgList<unsigned char> colors(1,1,1,1,1,
                                       0);
        CImg<float> opacities(1);
        opacities.fill(opacity);
        this->opacities.append(opacities);

    //    bool ok = vertices.is_object3d(primitives, colors, opacities);
    //    cout << "Check = " << ok << endl;

        this->vertices = this->vertices.append_object3d(this->primitives, vertices, primitives);

    //    CImg<unsigned char>().display_object3d("3d test", vertices, primitives, colors, opacities);
    //    image.draw_object3d(150,150,0, vertices, primitives, colors, opacities, 1);
    }
    void sphere(float radius = 1.0f, const vec3& pos = vec3()){
        mat3 mat = camera_mat();
        vec3 r_pos = mat*pos*1000.0f;

        CImgList<unsigned int> primitives;
        CImg<float> vertices = CImg<float>::sphere3d(primitives,radius*1000,2).shift_object3d(r_pos.x, r_pos.y, r_pos.z);

        unsigned v_count_1 = this->primitives.size();
        this->vertices = this->vertices.append_object3d(this->primitives, vertices, primitives);
        unsigned v_count_2 = this->primitives.size();

        unsigned diff = v_count_2 - v_count_1;
        CImg<float> opacities(diff);
        opacities.fill(0.5f);
        this->opacities.append(opacities);
    }
    void upper_half_sphere(float radius = 1.0f, const vec3& pos = vec3()){
        mat3 mat = camera_mat();
        vec3 r_pos = mat*pos*1000.0f;
        float r_radius = radius*1000;

        CImgList<unsigned int> primitives;
        CImg<float> vertices;

        int n1 = 5;
        int n2 = 20;
        for(int i=0; i<=n1; ++i){
            float psi = M_PI_2f32/n1*i;
            // not pole
            for(int j=0; j<n2; ++j){
                float phi = 2.0f*M_PIf32/n2*j;
                float z = pos.z + radius*sin(psi);
                float rr = radius*cos(psi);
                float x = pos.x + rr*cos(phi);
                float y = pos.y + rr*sin(phi);
                vec3 r_vec = mat*vec3(x,y,z)*1000.0f;
                CImg<float> vec(1,3,1,1, r_vec.x, r_vec.y, r_vec.z);
                vertices.append(vec);
                // skip last row
                if(i<n1){
                    int n = vertices.width()-1;
                    CImg<unsigned int> tri;
                    if(j<n2-1){
                        tri.assign(1,3,1,1, n+1,n+n2, n+1+n2);
                        primitives.push_back(tri);
                        tri.assign(1,3,1,1, n,n+n2,n+1);
                        primitives.push_back(tri);
                    }
                    // edge case: j==n2-1
                    else {
                        tri.assign(1,3,1,1, n-n2+1,n+n2, n+1);
                        primitives.push_back(tri);
                        tri.assign(1,3,1,1, n,n+n2,n-n2+1);
                        primitives.push_back(tri);
                    }// else
                }
            }// for j
        }// for i

        unsigned v_count_1 = this->primitives.size();
        this->vertices = this->vertices.append_object3d(this->primitives, vertices, primitives);
        unsigned v_count_2 = this->primitives.size();

        unsigned diff = v_count_2 - v_count_1;
        CImg<float> opacities(diff);
        opacities.fill(0.5f);
        this->opacities.append(opacities);
    }
    void point(const vec3& pos){
        sphere(0.01, pos);
    }
    void quad(const vec3& p1, const vec3& p2, const vec3& p3, const vec3& p4){}
    void axes(){
        vector(vec3(-1, 0, 0), vec3(2.0f,0,0), 0.5f);
        vector(vec3(0, -1, 0), vec3(0,2.0f,0), 0.5f);
        vector(vec3(0, 0, -1), vec3(0,0,2.0f), 0.5f);
        point(vec3(1.05f,0,0));
        point(vec3(0,1.05f,0));
        point(vec3(0,0,1.05f));
    }
};

TEST_CASE_METHOD(FigFixture, "Basic checks for DDFs"){

    SECTION("SphericalDdf"){
        SphericalDdf sd;
        REQUIRE(eq(sd.max_value, 0.25f/M_PI));
        REQUIRE(!sd.isSingular());
        REQUIRE(eq(sd.value(normalize(vec3(1,1,1))),0.25f/M_PI));
        REQUIRE(eq(sd.value(normalize(vec3(1,1,-1))), 0.25f/M_PI));
    }

    SECTION("UpperHalfDdf"){
        UpperHalfDdf ud;
        REQUIRE(eq(ud.max_value,0.5f/M_PI));
        REQUIRE(!ud.isSingular());
        upper_half_sphere();
        vector(vec3(), vec3(0,0,1));
        vector(vec3(), vec3(1,1,1));
        vector(vec3(), vec3(1,1,-1));
        // |
        // V
        REQUIRE(eq(ud.value(vec3(0,0,1)),0.5f/M_PI));
        REQUIRE(eq(ud.value(normalize(vec3(1,1,1))),0.5f/M_PI));
        REQUIRE(ud.value(normalize(vec3(1,1,-1)))==0.0f);

        display();
    }

    SECTION("CosineDdf"){
        CosineDdf cd;
        REQUIRE(eq(cd.max_value, 1.0f/M_PI));
        REQUIRE(!cd.isSingular());

        sphere(0.5f, vec3(0.0f,0.0f,0.5f));
        vector(vec3(), vec3(0,0,1));
        vector(vec3(), vec3(1,0,EPS/10.0f));
        vector(vec3(), normalize(vec3(1,1,-1)));
        // |
        // V
        REQUIRE(eq(cd.value(vec3(0,0,1)),M_1_PI));
        REQUIRE(eq(cd.value(vec3(1,0,EPS/10.0f)),0.0f));
        REQUIRE(cd.value(normalize(vec3(1,1,-1)))==0.0f);

        display();
    }

    SECTION("MirrorDdf"){
        MirrorDdf md;
        REQUIRE(isinf(md.max_value));
        REQUIRE(md.isSingular());
    }
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
