#include "GridRenderPlane.h"
#include "SimpleCamera.h"
#include "CollectionLighting.h"
#include "GeometrySphereInBox.h"
#include "GeometryFloor.h"
#include "GeometryOpenSpheres.h"

#include <sampling/ddf.h>
#include "tracer_interfaces.h"
#include <randf.h>

#include <glm/geometric.hpp>

#include <memory>
#include <iostream>

#include <cstdio>

using namespace glm;
using namespace std;

float ray_power(const Geometry& geometry, const Lighting& lighting, vec3 origin, vec3 direction, size_t depth=0){

    if(depth==4)
        return 0.0f;

    std::optional<surface_intersection> si = geometry.traceRay(origin, direction);
    std::optional<light_intersection> li   = lighting.traceRayToLight(origin, direction);

    // 1 check light hit
    if(li.has_value() && depth>0){
        // if not obscured by geometry
        if(!si.has_value() || length(si->position-origin) > length(li->position-origin))
            return li->surface_power;
    }// if li

    if(!si.has_value())
        return 0.0f;

    // 2 continue to geometry
    //DEBUG for geometry debugging
    //return si->position.y+1.0f;

    vec3 new_direction = si->sdf->trySample();

    if(new_direction == vec3())
        return 0.0f;            // possible dimming because of this

    return si->sdf->full_theoretical_weight * ray_power(geometry, lighting, si->position, new_direction, depth+1);
}

void render(const Scene& scene, RenderPlane& r_plane, size_t n_samples){
    for(size_t sample=0; sample<n_samples; ++sample){

        float x = randf();
        float y = randf();
//        for(float y = 0.5f/640; y < 1; y+=1.0f/640){
//        for(float x = 0.5f/640; x < 1; x+=1.0f/640){

        vec3 origin, direction;
        tie(origin, direction) = scene.camera->sampleRay(x, y);

        // ray bouncing recursion
        // with hard-limited depth
        r_plane.addRay(x, y, ray_power(*scene.geometry, *scene.lighting, origin, direction));

        // geometry hit
/*
        // 1 cast ray to light
        shared_ptr<const Ddf> light_ddf = scene.lighting->distributionInPoint(si->position);
        shared_ptr<const Ddf> combined_ddf = ::chain(light_ddf, si->sdf);

        vec3 light_direction = combined_ddf->trySample();
        if( light_direction == vec3())
            r_plane.addRay(x, y, 0.0f);
        else {

            light_intersection light_li   = Lighting::last_sample;
            std::optional<surface_intersection> light_si = scene.geometry->traceRay(si->position, light_direction);

            // if not obscured by geometry
            if(!light_si.has_value() || length(light_si->position-si->position) > length(light_li.position-si->position)){

                // NB We ignore surface_power and distance as they are already included in sampling function!
                float cosinus = dot(light_li.normal, -light_direction);
                if(cosinus < 0.0f)
                    cosinus = 0.0f;

                float value = cosinus * combined_ddf->full_theoretical_weight;

                r_plane.addRay(x, y, value);

            }// if not obscured by geometry

        }// if light is successfull
*/

        if((sample+1) % 10000 == 0)
            cout << (sample+1)/1000 << "k / " << n_samples/1000 << "k" << endl;

//        }}// for x y

//        cout << sample << " / " << n_samples << endl;

    }// for sample
}

int main(){

    shared_ptr<CollectionLighting> lighting = make_shared<CollectionLighting>();

    /*
    lighting->addPointLight(vec3{0.9f, 0.0f, -0.8f}, 0.1f, 1.0f);
    // TODO Why it has non-proportional power?
    lighting->addSphereLight(vec3{-0.8f, 0.0f, -0.8f}, 0.1f, 20.0f);
    // radiates down
    lighting->addAreaLight(vec3{-0.1f, +0.8f-0.1f, -0.8f}, vec3(0.0f, 0.0f, -1.0f), vec3{0.0f, 0.2f, 0.0f}, 1.0f);
    // radiates forward
    lighting->addAreaLight(vec3{-0.1f, -0.8f, -0.80-0.1f}, vec3(0.0f, 1.0f, 0.0f), vec3{0.0f, 0.0f, 0.2f}, 1.0f);
    */

    lighting->addOuterLight(10);

    shared_ptr<Geometry> geometry = make_shared<GeometryOpenSpheres>();
    vec3 camera_pos(0.0f, -3.0f, 0.1f);
    vec3 camera_dir = normalize(vec3(0.0f, 1.0f, -1.0f)-camera_pos);
    shared_ptr<SimpleCamera> camera = make_shared<SimpleCamera>( camera_pos, camera_dir );

    Scene scene{geometry, lighting, camera};

    GridRenderPlane r_plane(640, 640);

    render(scene, r_plane, 5*r_plane.width*r_plane.height);

    cout << "Max value = " << r_plane.max_value << endl;

    //r_plane.smooth(4);
    //r_plane.computeSmoothedMax(4);

    cout << "Smoothed max = " << r_plane.max_value << endl;

    FILE* fp = fopen("result.pgm", "wb");
    fprintf(fp, "P2\n%lu %lu\n%d\n", r_plane.width, r_plane.height, 255);

    float gamma = 0.6f;
    for(size_t y = 0; y<r_plane.height; ++y){
        for(size_t x = 0; x<r_plane.width; ++x){
            int value = pow(r_plane.pixels[y*r_plane.width+x]/r_plane.max_value, gamma)*255;
            if(value>255)
                value = 255;
            fprintf(fp, "%d ", value);
        }
        fprintf(fp, "\n");
    }// for y
    fclose(fp);

    return 0;
}
