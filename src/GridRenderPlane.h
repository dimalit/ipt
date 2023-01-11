#ifndef GRIDRENDERPLANE_H
#define GRIDRENDERPLANE_H

#include "tracer_interfaces.h"

#include <vector>

struct GridRenderPlane: public RenderPlane {
    std::vector<float> pixels;
    std::vector<size_t> pixel_counters;
    size_t width, height;
    float max_value = 0;

    GridRenderPlane(size_t width, size_t height);
    void smooth(size_t side);
    void computeSmoothedMax(size_t side);
    virtual void addRay(float x, float y, float value);
};
#endif // GRIDRENDERPLANE_H
