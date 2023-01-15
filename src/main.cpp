#include "GridRenderPlane.h"
#include "SimpleCamera.h"
#include "CollectionLighting.h"
#include "GeometrySphereInBox.h"
#include "GeometryFloor.h"
#include "GeometryOpenSpheres.h"

#include <sampling/sampling.h>
#include "tracer_interfaces.h"
#include <randf.h>

#include <glm/geometric.hpp>

#include <memory>
#include <iostream>

#include <cstdio>

using namespace glm;
using namespace std;

// TODO light_hint is not very good solution!
float ray_power(const Geometry& geometry, const Lighting& lighting, vec3 origin, vec3 direction, size_t depth=0, bool light_hint=false){

    if(depth==4)
        return 0.0f;

    std::optional<surface_intersection> si = geometry.traceRay(origin, direction);
    std::optional<light_intersection> li   = light_hint ? Lighting::last_sample : lighting.traceRayToLight(origin, direction);

    // 1 check light hit
    if(li.has_value() && depth>0){
        // if not obscured by geometry
        if(!si.has_value() || length(si->position-origin) > length(li->position-origin))
            assert(isnan(li->surface_power) || li->surface_power >= 0.0f);
            // HACK For point light
            return isfinite(li->surface_power) ? li->surface_power : 1.0f;
    }// if li

    if(!si.has_value())
        return 0.0f;

    // 2 continue to geometry
    //DEBUG for geometry debugging
    //return si->position.y+1.0f;

    shared_ptr<const Ddf> light_ddf = lighting.distributionInPoint(si->position);
    shared_ptr<const Ddf> chained_ddf = chain(light_ddf, si->sdf);

    // Mix equally SDF distribution and light*SDF distribution!
    // TODO Mix it with unite()!?
    vec3 new_direction;
    if(rand()%2==0)
        new_direction = light_ddf->trySample();
    else
        new_direction = si->sdf->trySample();

    if(new_direction == vec3())
        return 0.0f;            // possible dimming because of this

    assert(si->sdf->full_theoretical_weight >= 0.0f);

    return si->sdf->full_theoretical_weight * ray_power(geometry, lighting, si->position, new_direction, depth+1, true);
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
        float value = ray_power(*scene.geometry, *scene.lighting, origin, direction);
        // TODO investigate why it can be -1e-3
        assert(value > -1e-3);
        value = value >= 0.0f ? value : 0.0f;
        r_plane.addRay(x, y, value);

        if((sample+1) % 10000 == 0)
            cout << (sample+1)/1000 << "k / " << n_samples/1000 << "k" << endl;

//        }}// for x y

//        cout << sample << " / " << n_samples << endl;

    }// for sample
}

int main(){

    shared_ptr<CollectionLighting> lighting = make_shared<CollectionLighting>();

    lighting->addPointLight(vec3{0.9f, 0.0f, -0.8f}, 0.1f, 1.0f);
    // TODO Why it has non-proportional power?
    lighting->addSphereLight(vec3{-0.8f, 0.0f, -0.8f}, 0.1f, 1.0f);
    // radiates down
    lighting->addAreaLight(vec3{-0.1f, +0.8f-0.1f, -0.8f}, vec3(0.0f, 0.0f, -1.0f), vec3{0.0f, 0.2f, 0.0f}, 1.0f);
    // radiates forward
    lighting->addAreaLight(vec3{-0.1f, -0.8f, -0.80-0.1f}, vec3(0.0f, 1.0f, 0.0f), vec3{0.0f, 0.0f, 0.2f}, 1.0f);

    //lighting->addOuterLight(10);

    shared_ptr<Geometry> geometry = make_shared<GeometrySphereInBox>();
    vec3 camera_pos(0.0f, -3.0f, 0.1f);
    vec3 camera_dir = normalize(vec3(0.0f, 1.0f, -1.0f)-camera_pos);
    shared_ptr<SimpleCamera> camera = make_shared<SimpleCamera>( camera_pos, camera_dir );

    Scene scene{geometry, lighting, camera};

    GridRenderPlane r_plane(640, 640);

    render(scene, r_plane, 50*r_plane.width*r_plane.height);

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
