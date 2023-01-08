#include "LightingImpl.h"
#include "geometry.h"

#include "tracer_interfaces.h"

#include <memory>
#include <iostream>

#include <cstdio>

using namespace glm;
using namespace std;

struct GridRenderPlane: public RenderPlane {
    vector<float> pixels;
    vector<size_t> pixel_counters;
    size_t width, height;
    float max_value = 0;

    GridRenderPlane(size_t width, size_t height){
        this->width = width;
        this->height = height;
        pixels.resize(width*height);
        pixel_counters.resize(width*height);
    }
    virtual void addRay(float x, float y, float value) override {
        assert(x >= 0.0f && x < 1.0f);
        assert(y >= 0.0f && y < 1.0f);

        size_t xi = x*width;
        size_t yi = height-y*height-1;
        pixels[yi*width+xi] =
                (pixels[yi*width+xi]*pixel_counters[yi*width+xi]+value)
                /
                (pixel_counters[yi*width+xi]+1);
        ++pixel_counters[yi*width+xi];
        if(pixels[yi*width+xi] > max_value)
            max_value = pixels[yi*width+xi];
    }
};

struct SimpleCamera: public Camera {
    glm::vec3 position;
    glm::vec3 direction;

    SimpleCamera(glm::vec3 position, glm::vec3 direction){
        this->position = position;
        this->direction = direction;
    }

    virtual std::pair<glm::vec3, glm::vec3> sampleRay(float x, float y) const override {
        x -= 0.5f;
        y -= 0.5f;
        vec3 right = normalize(cross(direction, vec3(0.0f, 0.0f, 1.0f)));
        vec3 up = normalize(cross(right, direction));

        vec3 ray = right*x + up*y + direction;
        return {position, normalize(ray)};
    }
};


#include <random>

// TODO deduplicate
static struct{
    std::random_device rdev;
    std::mt19937 gen = std::mt19937(rdev());

    std::uniform_real_distribution<> dist = std::uniform_real_distribution<>(0.0f, 1.0f);

    float operator()() {
        float res = dist(gen);
        while(res==1.0f)
            res = dist(gen);
        return res;
    }
} randf;

void render(const Scene& scene, RenderPlane& r_plane, size_t n_samples){
    for(size_t sample=0; sample<n_samples; ++sample){

        float x = randf();
        float y = randf();

        vec3 origin, direction;
        tie(origin, direction) = scene.camera->sampleRay(x, y);

        // ray bouncing loop
        // with hard-limited depth
        float dimming_coef = 1.0f;
        for(size_t depth=0; depth<1; ++depth){

            std::optional<surface_intersection> si = scene.geometry->traceRay(origin, direction);
            std::optional<light_intersection> li   = scene.lighting->traceRayToLight(origin, direction);

            // light is our 1st priority!
            if(li.has_value()){
                // if not obscured by geometry
                if(!si.has_value() || length(si->position-origin) > length(li->position-origin)){
                    float value = dot(li->normal, -direction)*li->surface_power;
                    r_plane.addRay(x, y, value*dimming_coef);
                    break;
                }
            }// if li

            if(!si.has_value()){
                r_plane.addRay(x, y, 0.0f);
                break;
            }

            // geometry hit

            // 1 cast ray to light
            shared_ptr<const Ddf> light_ddf = scene.lighting->distributionInPoint(si->position);
            shared_ptr<const Ddf> combined_ddf = ::apply(light_ddf, si->sdf);
            vec3 light_direction = combined_ddf->trySample();

            std::optional<surface_intersection> light_si = scene.geometry->traceRay(si->position, light_direction);
            std::optional<light_intersection> light_li   = scene.lighting->traceRayToLight(si->position, light_direction);
            if(light_li.has_value()){
                // if not obscured by geometry
                if(!light_si.has_value() || length(light_si->position-si->position) > length(light_li->position-si->position)){
                    float value = dot(light_li->normal, -light_direction)*light_li->surface_power/pow(length(light_li->position-si->position), 2);
                    r_plane.addRay(x, y, value);
                }
            }// if li

            // 2 continue to geometry
            vec3 new_direction = si->sdf->trySample();

            if(new_direction == vec3()){
                r_plane.addRay(x, y, 0.0f);
                break;
            }

            origin = si->position;
            direction = new_direction;
            dimming_coef *= si->sdf->full_theoretical_weight;
            continue;

        }// ray bouncing loop

        // rays that fell above depth limit are just ignored

        if((sample+1) % 10000 == 0)
            cout << (sample+1)/1000 << "k / " << n_samples/1000 << "k" << endl;

    }// for sample
}

int main(){

    LightingImpl* lighting = new LightingImpl();
    //lighting->lights.push_back(make_shared<const PointLight>(vec3{1.0f, 0.0f, -0.9f}, 10.0f));
    // TODO Why it has non-proportional power?
    lighting->lights.push_back(make_shared<const SphereLight>(vec3{-1.0f, 0.0f, -0.80f}, 10.0f, 0.1f));
    // radiates down
    lighting->lights.push_back(make_shared<const AreaLight>(vec3{-0.1f, +1.0f-0.1f, -0.8f}, vec3{0.0f, 0.2f, 0.0f}, vec3{0.2f, 0.0f, 0.0f}, 1.0f));
    // radiates forward
    lighting->lights.push_back(make_shared<const AreaLight>(vec3{-0.1f, -1.0f, -0.80-0.1f}, vec3{0.0f, 0.0f, 0.2f}, vec3{0.2f, 0.0f, 0.0f}, 1.0f));

    Geometry* geometry = new Floor();
    vec3 camera_pos(0.0f, -3.0f, 0.1f);
    vec3 camera_dir = normalize(vec3(0.0f, 0.0f, -1.0f)-camera_pos);
    SimpleCamera* camera = new SimpleCamera( camera_pos, camera_dir );

    Scene scene{std::shared_ptr<const Geometry>(geometry), std::shared_ptr<const Lighting>(lighting), std::shared_ptr<const Camera>(camera)};

    GridRenderPlane r_plane(640, 640);

    render(scene, r_plane, 5*r_plane.width*r_plane.height);

    cout << "Max value = " << r_plane.max_value << endl;

    FILE* fp = fopen("result.pgm", "wb");
    fprintf(fp, "P2\n%lu %lu\n%d\n", r_plane.width, r_plane.height, 255);

    float gamma = 1.0f;
    for(size_t y = 0; y<r_plane.height; ++y){
        for(size_t x = 0; x<r_plane.width; ++x){
            fprintf(fp, "%d ", (int)(pow(r_plane.pixels[y*r_plane.width+x]/r_plane.max_value, gamma)*255));
        }
        fprintf(fp, "\n");
    }// for y
    fclose(fp);

    return 0;
}
