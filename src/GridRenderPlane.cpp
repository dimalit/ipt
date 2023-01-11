#include "GridRenderPlane.h"

GridRenderPlane::GridRenderPlane(size_t width, size_t height){
    this->width = width;
    this->height = height;
    pixels.resize(width*height);
    pixel_counters.resize(width*height);
}

void GridRenderPlane::smooth(size_t side){

    float max_value = 0.0f;

    for(size_t y = height-1; y>=side-1; --y){
        for(size_t x = width-1; x>=side-1; --x){

            // summing loop
            // TODO optimize passes
            float accum = 0.0f;
            for(size_t yy=0; yy<side; ++yy){
                for(size_t xx=0; xx<side; ++xx){
                    size_t i = (y-yy)*width+x-xx;
                    accum += pixels[i];
                }
            }
            accum /= side*side;
            pixels[y*width+x] = accum;
            if(accum > max_value)
                max_value = accum;
        }// x
    }// y

    this->max_value = max_value;
}

void GridRenderPlane::computeSmoothedMax(size_t side){

    float max_value = 0.0f;

    for(size_t y = height-1; y>=side-1; --y){
        for(size_t x = width-1; x>=side-1; --x){

            // summing loop
            // TODO optimize passes
            float accum = 0.0f;
            for(size_t yy=0; yy<side; ++yy){
                for(size_t xx=0; xx<side; ++xx){
                    size_t i = (y-yy)*width+x-xx;
                    accum += pixels[i];
                }
            }
            accum /= side*side;
            if(accum > max_value)
                max_value = accum;
        }// x
    }// y

    this->max_value = max_value;
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
