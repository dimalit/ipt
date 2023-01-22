#include "check_ddf.h"

#include <sampling/sampling.h>
#include <lighting/lighting.h>

#include <iostream>
#include <vector>

#include <cstring>

using namespace glm;
using namespace std;

void test_light_ddfs(){
    InvertedSphereLight isl(vec3(0.0f, 0.0f, -0.5f), 1.0f, 1.0f);
    shared_ptr<const Ddf> ild = isl.lightToPoint(vec3(0.0f, 0.0f, 0.0f));
    cout << "InvertedSphereLight:" << endl;
    cout << (check_ddf(*ild) ? "OK" : "FAIL") << endl;
    cout << endl;

    AreaLight al(vec3(-1.0f, -1.0f, 1.0f), vec3(0.0f, 2.0f, 0.0f), vec3(2.0f, 0.0f, 0.0f), 1.0f);
    shared_ptr<const Ddf> ld = al.lightToPoint(vec3(0.0f, 0.0f, 0.0f));
    cout << "AreaLight 2x2 z=1 above:" << endl;
    cout << (check_ddf(*ld) ? "OK" : "FAIL") << endl;
    cout << endl;

    SphereLight sl(vec3(0.0f, 0.0f, 1.3f), 1.0f, 1.0f);
    shared_ptr<const Ddf> cd = make_shared<CosineDdf>();
    shared_ptr<const Ddf> comb = chain(ld, cd);

    cout << "SphereLight:" << endl;
    cout << (check_ddf(*ld) ? "OK" : "FAIL") << endl;
    cout << endl;
}

void test_singulars(){
    shared_ptr<const Ddf> mirror = make_shared<MirrorDdf>();
    assert(mirror->trySample()==vec3(0.0f, 0.0f, 1.0f));

    PointLight p_light(vec3(0.0f, 0.0f, 1.0f), 0.1f);
    assert(p_light.area==0);
    assert(p_light.sample().position==vec3(0.0f, 0.0f, 1.0f));

    shared_ptr<const Ddf> c_ddf = make_shared<CosineDdf>();
    SphereLight s_light(vec3(0.0f, 0.0f, 1.3f), 1.0f);

    shared_ptr<const Ddf> point2cosine = chain(p_light.lightToPoint(vec3()), c_ddf);
    shared_ptr<const Ddf> sphere2mirror = chain(s_light.lightToPoint(vec3()), mirror);

    // TODO check that point2mirror will assert!
    //shared_ptr<const Ddf> point2mirror = chain(p_light.lightToPoint(vec3()), mirror);

    vector<shared_ptr<const Ddf>> vec{point2cosine, sphere2mirror};
    for(shared_ptr<const Ddf> e: vec){
        vec3 res;
        do{
            res = e->trySample();
        }while(res==vec3());
        cout << res.x << " " << res.y << " " << res.z << endl;
    }
}

int main(){

    test_light_ddfs();
    test_singulars();

    return 0;
}
