#include "sample_scenes.h"

#include "SimpleCamera.h"
#include "CollectionLighting.h"

#include <geometry/GeometrySphereInBox.h>
#include <geometry/GeometryFloor.h>
#include <geometry/GeometryOpenSpheres.h>
#include <geometry/FractalSpheres.h>
#include <geometry/GeometrySmallPt.h>
#include <geometry/GeometryCorner.h>

#include <glm/geometric.hpp>

#include <memory>

using namespace glm;
using namespace std;

Scene make_scene_box(){

    shared_ptr<CollectionLighting> lighting = make_shared<CollectionLighting>();

    //lighting->addPointLight(vec3{0.9f, 0.0f, -0.8f}, 0.1f, 1.0f);
    // TODO Why it has non-proportional power?
    //lighting->addSphereLight(vec3{-0.8f, 0.0f, -0.8f}, 0.1f, 1.0f);
    // radiates down
    lighting->addSquareLight(vec3{+0.1f, -0.8f-0.1f, -0.15f}, vec3(0.0f, 0.0f, +1.0f), vec3{0.0f, 0.2f, 0.0f}, 1.0f);
    // radiates forward
    //lighting->addSquareLight(vec3{-0.1f, -0.8f, -0.7f-0.1f}, vec3(0.0f, 1.0f, 0.0f), vec3{0.0f, 0.0f, 0.2f}, 1.0f);

    //lighting->addOuterLight(10);

    shared_ptr<Geometry> geometry = make_shared<GeometrySphereInBox>();

    vec3 camera_pos(0.0f, -3.0f, 0.1f);
    vec3 camera_dir = normalize(vec3(0.0f, 1.0f, -1.0f)-camera_pos);
    shared_ptr<SimpleCamera> camera = make_shared<SimpleCamera>( camera_pos, camera_dir );

    return Scene{geometry, lighting, camera};
}

Scene make_scene_fractal(){
    shared_ptr<CollectionLighting> lighting = make_shared<CollectionLighting>();
    lighting->addSphereLight(vec3(-5.5,0,0), 1);

    shared_ptr<Geometry> geometry = make_shared<FractalSpheres>();

    vec3 camera_pos(0.0f, -4.0f, 0.0f);
    vec3 camera_dir = vec3(0,1,0);
    shared_ptr<SimpleCamera> camera = make_shared<SimpleCamera>( camera_pos, camera_dir );

    return Scene{geometry, lighting, camera};
}

Scene make_scene_smallpt(){

    shared_ptr<CollectionLighting> lighting = make_shared<CollectionLighting>();

    vec3 lc = vec3(50,81.6-16.5,81.6);
    //lighting->addSphereLight(lc, 1.5f);   // small sphere
    //lighting->addSphereLight(vec3(50,681.6-.27,81.6f), 600.0f);   // big sphere
    lighting->addSquareLight(lc-vec3(4.0f, 0, 4.0f), vec3(0,-1,0), vec3(8.0f,0,0));

    //lighting->addSphereLight(vec3(50,50,50), 10.0f);
    //lighting->addOuterLight(1e+4);

    shared_ptr<Geometry> geometry = make_shared<GeometrySmallPt>();

    vec3 camera_pos(50.0f,52.0f,295.6f);
    vec3 camera_dir = normalize(vec3(0.0f,-0.042612f,-1.0f));
    shared_ptr<SimpleCamera> camera = make_shared<SimpleCamera>( camera_pos, camera_dir*2.0f, vec3(0,1,0) );

    return Scene{geometry, lighting, camera};
}


 Scene make_scene_square_lit_by_square(){

    shared_ptr<CollectionLighting> lighting = make_shared<CollectionLighting>();
     lighting->addSquareLight(vec3(-0.05f,-0.05f,-0.9f), vec3(0,0,-1), vec3(0,0.1f,0));

    shared_ptr<Geometry> geometry = make_shared<GeometryFloor>();

    vec3 camera_pos(0, -5.0f, 0);
    vec3 camera_dir = normalize(vec3(0,0,-1.0f)-camera_pos);
    shared_ptr<SimpleCamera> camera = make_shared<SimpleCamera>( camera_pos, camera_dir*2.0f, vec3(0,1,0) );

    return Scene{geometry, lighting, camera};
}

 Scene make_scene_lit_corner(){

     shared_ptr<CollectionLighting> lighting = make_shared<CollectionLighting>();
     vec3 out = vec3(1,1,1);
     vec3 cx = vec3(-0.5f, -1.0f, -1.0f)+0.5f*out;
     vec3 cy = vec3(-1.0f, -0.5f, -1.0f)+0.5f*out;
     vec3 cz = vec3(-1.0f, -1.0f, -0.5f)+0.5f*out;
     lighting->addTriangleLight(cx, cz-cx, cy-cx);

     shared_ptr<Geometry> geometry = make_shared<GeometryCorner>();

     vec3 camera_pos(4.0, 1.0f, 1.0f);
     vec3 camera_dir = normalize(vec3(0,0,0.0f)-camera_pos);
     shared_ptr<SimpleCamera> camera = make_shared<SimpleCamera>( camera_pos, camera_dir );

     return Scene{geometry, lighting, camera};
 }
