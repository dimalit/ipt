#include "SimpleCamera.h"

#include <glm/geometric.hpp>

using namespace glm;
using namespace std;

pair<vec3, vec3> SimpleCamera::sampleRay(float x, float y) const {
    x -= 0.5f;
    y -= 0.5f;
    vec3 right = normalize(cross(direction, vec3(0.0f, 0.0f, 1.0f)));
    vec3 up = normalize(cross(right, direction));

    vec3 ray = right*x + up*y + direction;
    return {position, normalize(ray)};
}
