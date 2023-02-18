#ifndef SIMPLECAMERA_H
#define SIMPLECAMERA_H

#include "tracer_interfaces.h"

#include <glm/vec3.hpp>

#include <utility>

struct SimpleCamera: public Camera {
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 right, up;

    SimpleCamera(glm::vec3 position, glm::vec3 direction, glm::vec3 up_hint = glm::vec3(0,0,1));
    virtual std::pair<glm::vec3, glm::vec3> sampleRay(float x, float y) const override;
};

#endif // SIMPLECAMERA_H
