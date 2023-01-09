#include "GridRenderPlane.h"

GridRenderPlane::GridRenderPlane(size_t width, size_t height){
    this->width = width;
    this->height = height;
    pixels.resize(width*height);
    pixel_counters.resize(width*height);
}
void GridRenderPlane::addRay(float x, float y, float value){
    assert(x >= 0.0f && x < 1.0f);
    assert(y >= 0.0f && y < 1.0f);
    assert(value >= 0.0f);

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
