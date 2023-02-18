#include "SimpleCamera.h"

#include <glm/geometric.hpp>

using namespace glm;
using namespace std;

SimpleCamera::SimpleCamera(vec3 position, vec3 direction, vec3 up_hint){
    this->position = position;
    this->direction = direction;
    right = normalize(glm::cross(direction, up_hint));
    up = normalize(glm::cross(right, direction));
}

pair<vec3, vec3> SimpleCamera::sampleRay(float x, float y) const {
    x -= 0.5f;
    y -= 0.5f;

    vec3 ray = right*x + up*y + direction;
    return {position, normalize(ray)};
}
