#include "GridRenderPlane.h"
#include "gui.h"

#include "sample_scenes.h"

#include <libddf/ddf.h>
#include <randf.h>

#include "tracer_interfaces.h"

#include <glm/geometric.hpp>

#include <memory>
#include <iostream>
#include <thread>
#include <atomic>

#include <cstdio>

using namespace glm;
using namespace std;

// note: root will manage memory for all children
class StatsNode {
public:
    void put(std::string key, std::string value){}
    void put(std::string key, bool value){}
    void put(std::string key, int64_t value){}
    void put(std::string key, uint64_t value){}
    void put(std::string key, float value){}
    void put(std::string key, glm::vec3 value){}
    void addChild(std::string key, StatsNode* child){}
    ~StatsNode(){
        // delete children
    }
};

size_t n_rays = 5;
size_t depth_max = 3;

// TODO light_hint is not very good solution!
float ray_power(const Geometry& geometry, const Lighting& lighting, vec3 origin, vec3 direction, size_t depth, StatsNode* stats){

    if(depth==depth_max)
        return 0.0f;

    // Stats: origin, depth
    stats->put("origin", origin);
    stats->put("depth", depth);

    std::optional<surface_intersection> si = geometry.traceRay(origin, direction);
    std::optional<light_intersection> li   = lighting.traceRayToLight(origin, direction);

    // 1 check light hit
    if(li.has_value()){
        // if not obscured by geometry
        if(!si.has_value() || length(si->position-origin) > length(li->position-origin)){

            // Stats: li, true (light hit)
            stats->put("li", true);
            stats->put("li_position", li->position);
            stats->put("li_power", li->surface_power);
            stats->put("li_normal", li->normal);

            assert(isnan(li->surface_power) || li->surface_power >= 0.0f);
            // HACK For point light
            return isfinite(li->surface_power) ? li->surface_power : 1.0f;
        }
    }// if li

    if(!si.has_value())
        return 0.0f;

    // Stats: si, true (surface hit)
    stats->put("si", true);
    stats->put("si_position", si->position);
    stats->put("si_albedo", si->albedo);

    // 2 continue to geometry
    //DEBUG for geometry debugging
    //return si->position.y+1.0f;

    // TODO better solution?
    Ddf* sdf_tmp = si->sdf.get();

    unique_ptr<Ddf> light_ddf = lighting.distributionInPoint(si->position);
    unique_ptr<Ddf> mix_ddf = unite(move(light_ddf), 1.0f, move(si->sdf), 1.0f);

    vec3 new_direction;
    float res = 0.0f;

    // Stats: child node
    for(size_t i=0; i<n_rays; ++i){

        new_direction = mix_ddf->sample();

        if( new_direction == vec3() ){
            continue;
        }

        // Stats: false (miss)
        StatsNode* child_stats = new StatsNode();
        stats->addChild("secondary_rays", child_stats);

        // to compute integral of ray_power*sdf using sampling from mix_ddf,
        // we need ray_power*sdf/mix_ddf
        // Stats: multiplier
        float multiplier = sdf_tmp->value(new_direction)/mix_ddf->value(new_direction);
        child_stats->put("multiplier", multiplier);
        assert(isfinite(multiplier));
        // Stats: child ray_power()
        res += multiplier*si->albedo * ray_power(geometry, lighting, si->position, new_direction, depth+1, child_stats);
    }

    // Stats: result
    res = isfinite(res) ? res/n_rays : 0.0f;
    stats->put("result", res);
    return res;
}

void render_sample(const Scene& scene, RenderPlane& r_plane, StatsNode* stats){
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

        StatsNode* pixel_stats = new StatsNode();
        pixel_stats->put("x", x);
        pixel_stats->put("y", y);

        vec3 origin, direction;
        tie(origin, direction) = scene.camera->sampleRay(x, y);
        pixel_stats->put("origin", origin);
        pixel_stats->put("direction", direction);

        // ray bouncing recursion
        // with hard-limited depth
        float value = ray_power(*scene.geometry, *scene.lighting, origin, direction, 0, pixel_stats);
        // TODO investigate why it can be -1e-3
        //assert(value > -1e-3);
        value = value >= 0.0f ? value : 0.0f;
        assert(isfinite(value));
        r_plane.addRay(x, y, value);

        stats->addChild("pixels", pixel_stats);

    }// for x
    //cout << iy << endl;
    }// for y
}//render_sample()

static int n_val(float val, float max, float contrast, float gamma){
    float adj = val*contrast/max;
    adj = std::min(adj, contrast);
    adj = std::max(adj, 1.0f);
    float fn = log(adj)/log(contrast);
    fn = pow(fn, gamma);
    int n = 256*fn;
    n = std::max(0, n);
    n = std::min(255, n);
    return n;
}

int main(int argc, char** argv){

    if(argc>1)
        n_rays = atoi(argv[1]);

    if(argc>2)
        depth_max = atoi(argv[2]);

    cout << "Rendering with " << n_rays << " rays per pixel and max depth " << depth_max << endl;

    Scene scene = make_scene_lit_corner();

    GridRenderPlane r_plane(640, 640);

    Gui gui;

    StatsNode stats;

    std::atomic<bool> termination_requested = false;
    std::thread th([&scene, &gui, &termination_requested, &stats](){
        // render infinitely
        for(size_t sample=0;; ++sample){
            StatsNode* sample_stats = new StatsNode();
            stats.addChild("samples", sample_stats);
            sample_stats->put("sample", sample);
            render_sample(scene, gui, sample_stats);
            cout << "Sample " << (sample+1) << endl;
            if(termination_requested)
                break;
        }// for sample
    });

    gui.work();

    termination_requested = true;
    th.join();

    gui.finalize();
    cout << "Saving to result.png" << endl;
    gui.save("result.png");

//    cout << "Max value = " << r_plane.max_value << endl;

//    //r_plane.smooth(2);
//    //r_plane.computeSmoothedMax(5);
//    cout << "Smoothed max = " << r_plane.max_value << endl;

//    FILE* fp = fopen("result.pgm", "wb");
//    fprintf(fp, "P2\n%lu %lu\n%d\n", r_plane.width, r_plane.height, 255);

//    float contrast = 500;
//    float gamma = 0.6f;
//    for(size_t y = 0; y<r_plane.height; ++y){
//        for(size_t x = 0; x<r_plane.width; ++x){
//            int ivalue = n_val(r_plane.pixels[y*r_plane.width+x], r_plane.max_value, contrast, gamma);
//            fprintf(fp, "%d ", ivalue);
//        }
//        fprintf(fp, "\n");
//    }// for y
//    fclose(fp);

    return 0;
}
