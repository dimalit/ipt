#ifndef GUI_H
#define GUI_H

#include "tracer_interfaces.h"

#include <CImg.h>

#include <vector>

// TODO Add const
// TODO Deduplicate some parts
class Gui: public RenderPlane {
private:
    cimg_library::CImg<float> image;
    cimg_library::CImgDisplay display;
    cimg_library::CImgDisplay hist_display;
    size_t addRay_counter = 0;
    std::vector<std::vector<size_t>> value_counter;
public:
    Gui();
    void work();
    virtual void addRay(float x, float y, float value);
    // Show all pending pixels
    void finalize();
    void save(const char* path);
private:
    void draw_text_overlay(cimg_library::CImg<float>& arg);
    static cimg_library::CImg<float> normalize(const cimg_library::CImg<float>& arg);
};

#endif // GUI_H
