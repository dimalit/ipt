#include "GridRenderPlane.h"
#include "SimpleCamera.h"
#include "CollectionLighting.h"

#include <geometry/GeometrySphereInBox.h>
#include <geometry/GeometryFloor.h>
#include <geometry/GeometryOpenSpheres.h>
#include <geometry/FractalSpheres.h>

#include <lighting/lighting.h>

#include <libddf/ddf.h>
#include "tracer_interfaces.h"
#include <randf.h>

#include <glm/geometric.hpp>

#include <memory>
#include <iostream>

#include <cstdio>

using namespace glm;
using namespace std;

std::mt19937 gen_for_depth_0;

// TODO light_hint is not very good solution!
float ray_power(const Geometry& geometry, const Lighting& lighting, vec3 origin, vec3 direction, size_t depth=0){

    if(depth==3)
        return 0.0f;

    std::optional<surface_intersection> si = geometry.traceRay(origin, direction);
    std::optional<light_intersection> li   = lighting.traceRayToLight(origin, direction);

    // 1 check light hit
    if(li.has_value() && depth>0){
        // if not obscured by geometry
        if(!si.has_value() || length(si->position-origin) > length(li->position-origin)){
            assert(isnan(li->surface_power) || li->surface_power >= 0.0f);
            // HACK For point light
            return isfinite(li->surface_power) ? li->surface_power : 1.0f;
        }
    }// if li

    if(!si.has_value())
        return 0.0f;

    // 2 continue to geometry
    //DEBUG for geometry debugging
    //return si->position.y+1.0f;

    // TODO better solution?
    Ddf* sdf_tmp = si->sdf.get();

    unique_ptr<Ddf> light_ddf = lighting.distributionInPoint(si->position);
    unique_ptr<Ddf> mix_ddf = unite(move(light_ddf), 1.0f, move(si->sdf), 1.0f);

    vec3 new_direction;
    float res = 0.0f;

    for(size_t i=0; i<10; ++i){
        new_direction = mix_ddf->trySample(depth==0 ? gen_for_depth_0 : gen);
        // possible dimming because of this
        if(new_direction != vec3()){
            // correct by light_ddf distribution!
            float multiplier = 1.0f/mix_ddf->value(new_direction)*sdf_tmp->value(new_direction);
            res += multiplier*si->albedo * ray_power(geometry, lighting, si->position, new_direction, depth+1);
        }
    }
    return isfinite(res)?res/10 :0.0f;
}

void render(const Scene& scene, RenderPlane& r_plane, size_t n_samples){
    for(size_t sample=0; sample<n_samples; ++sample){

//        float x = randf();
//        float y = randf();
        for(size_t iy = 0; iy < 640; iy++){
        for(size_t ix = 0; ix < 640; ix++){

        float x = (ix+randf())/640.0f;
        float y = (iy+randf())/640.0f;

        if(x==1.0f)
            x=nextafter(x, 0.0f);
        if(y==1.0f)
            y=nextafter(y, 0.0f);

        vec3 origin, direction;
        tie(origin, direction) = scene.camera->sampleRay(x, y);

        // ray bouncing recursion
        // with hard-limited depth
        gen_for_depth_0.seed();
        float value = ray_power(*scene.geometry, *scene.lighting, origin, direction);
        // TODO investigate why it can be -1e-3
        assert(value > -1e-3);
        value = value >= 0.0f ? value : 0.0f;
        assert(isfinite(value));
        r_plane.addRay(x, y, value);

        }// for x
        //cout << iy << endl;
        }// for y

        cout << "Sample " << sample << " / " << n_samples << " OK" << endl;

    }// for sample
}

Scene make_scene_box(){

    shared_ptr<CollectionLighting> lighting = make_shared<CollectionLighting>();

    //lighting->addPointLight(vec3{0.9f, 0.0f, -0.8f}, 0.1f, 1.0f);
    // TODO Why it has non-proportional power?
    //lighting->addSphereLight(vec3{-0.8f, 0.0f, -0.8f}, 0.1f, 1.0f);
    // radiates down
    lighting->addAreaLight(vec3{+0.1f, -0.8f-0.1f, -0.15f}, vec3(0.0f, 0.0f, -1.0f), vec3{0.0f, 0.2f, 0.0f}, 1.0f);
    // radiates forward
    //lighting->addAreaLight(vec3{-0.1f, -0.8f, -0.7f-0.1f}, vec3(0.0f, 1.0f, 0.0f), vec3{0.0f, 0.0f, 0.2f}, 1.0f);

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

int main(){

    Scene scene = make_scene_box();

    GridRenderPlane r_plane(640, 640);

    render(scene, r_plane, 10);

    cout << "Max value = " << r_plane.max_value << endl;

    //r_plane.smooth(2);
    r_plane.computeSmoothedMax(5);

    cout << "Smoothed max = " << r_plane.max_value << endl;

    FILE* fp = fopen("result.pgm", "wb");
    fprintf(fp, "P2\n%lu %lu\n%d\n", r_plane.width, r_plane.height, 255);

    float gamma = 0.6f;
    for(size_t y = 0; y<r_plane.height; ++y){
        for(size_t x = 0; x<r_plane.width; ++x){
            float value = r_plane.pixels[y*r_plane.width+x]/r_plane.max_value;
            if(value>1.0f)
                value = 1.0f;
            int ivalue = pow(value, gamma)*255;
            fprintf(fp, "%d ", ivalue);
        }
        fprintf(fp, "\n");
    }// for y
    fclose(fp);

    return 0;
}
