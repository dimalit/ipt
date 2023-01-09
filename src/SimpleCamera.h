#ifndef SIMPLECAMERA_H
#define SIMPLECAMERA_H

#include "tracer_interfaces.h"

#include <glm/vec3.hpp>

#include <utility>

struct SimpleCamera: public Camera {
    glm::vec3 position;
    glm::vec3 direction;

    SimpleCamera(glm::vec3 position, glm::vec3 direction){
        this->position = position;
        this->direction = direction;
    }

    virtual std::pair<glm::vec3, glm::vec3> sampleRay(float x, float y) const override;
};

#endif // SIMPLECAMERA_H
