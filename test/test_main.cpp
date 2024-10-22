#include "check_ddf.h"

#include <libddf/ddf_detail.h>
#include <lighting/lighting.h>

#include <iostream>
#include <vector>

#include <cstring>

using namespace glm;
using namespace std;

void test_light_ddfs(){
    InvertedSphereLight isl(vec3(0.0f, 0.0f, -0.5f), 1.0f, 1.0f);
    shared_ptr<Ddf> ild = isl.lightToPoint(vec3(0.0f, 0.0f, 0.0f));
    cout << "InvertedSphereLight:" << endl;
    cout << (check_ddf(*ild) ? "OK" : "FAIL") << endl;
    cout << endl;

    AreaLight al(vec3(-1.0f, -1.0f, 1.0f), vec3(0.0f, 2.0f, 0.0f), vec3(2.0f, 0.0f, 0.0f), 1.0f);
    unique_ptr<Ddf> ld = al.lightToPoint(vec3(0.0f, 0.0f, 0.0f));
    cout << "AreaLight 2x2 z=1 above:" << endl;
    cout << (check_ddf(*ld) ? "OK" : "FAIL") << endl;
    cout << endl;

    SphereLight sl(vec3(0.0f, 0.0f, 1.3f), 1.0f, 1.0f);
    ld = sl.lightToPoint(vec3(0.0f, 0.0f, 0.0f));
    unique_ptr<Ddf> cd = make_unique<CosineDdf>();
    //TODO Test it too!
    //unique_ptr<Ddf> comb = chain(move(ld), move(cd));

    cout << "SphereLight:" << endl;
    cout << (check_ddf(*ld) ? "OK" : "FAIL") << endl;
    cout << endl;
}

void test_singulars(){
    unique_ptr<Ddf> mirror = make_unique<MirrorDdf>();
    assert(mirror->sample()==vec3(0.0f, 0.0f, 1.0f));

    PointLight p_light(vec3(0.0f, 0.0f, 1.0f), 0.1f);
    assert(p_light.area==0);
    assert(p_light.sample().position==vec3(0.0f, 0.0f, 1.0f));

    unique_ptr<Ddf> c_ddf = make_unique<CosineDdf>();
    SphereLight s_light(vec3(0.0f, 0.0f, 1.3f), 1.0f);
}

int main(){

    test_light_ddfs();
    test_singulars();

    return 0;
}
