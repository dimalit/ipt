#include "LightingImpl.h"
#include "geometry.h"

#include "tracer_interfaces.h"

#include <memory>

using namespace glm;
using namespace std;

int main(){

    LightingImpl* lighting = new LightingImpl();
    lighting->lights.push_back(make_shared<const PointLight>(vec3{1.0f, 0.0f, 0.0f}, 1.0f));
    lighting->lights.push_back(make_shared<const SphereLight>(vec3{-1.0f, 0.0f, 0.0f}, 1.0f, 0.01f));
    lighting->lights.push_back(make_shared<const AreaLight>(vec3{-0.01f, -1.0f-0.01f, 0.0f}, vec3{0.0f, 0.02f, 0.0f}, vec3{0.02f, 0.0f, 0.0f}, 1.0f));
    lighting->lights.push_back(make_shared<const AreaLight>(vec3{-0.01f, +1.0f, -0.01f}, vec3{0.02f, 0.0f, 0.0f}, vec3{0.0f, 0.0f, 0.02f}, 1.0f));

    Geometry* geometry = new Floor();

    Scene scene{std::shared_ptr<const Geometry>(geometry), std::shared_ptr<const Lighting>(lighting), nullptr};

    return 0;
}
