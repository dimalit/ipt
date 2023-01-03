#ifndef GEOMETRIC_UTILS_H
#define GEOMETRIC_UTILS_H

#include <glm/vec3.hpp>

extern float intersection_with_box_plane(glm::vec3 plane, glm::vec3 origin, glm::vec3 direction);
extern float intersection_with_sphere(float radius, glm::vec3 origin, glm::vec3 direction);

#endif // GEOMETRIC_UTILS_H
