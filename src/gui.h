#ifndef GUI_H
#define GUI_H

#include "pch.h"

#include "tracer_interfaces.h"
#include "SimpleCamera.h"

#include <vector>
#include <mutex>

// TODO Add const
// TODO Deduplicate some parts
class Gui: public RenderPlane, public Camera {
private:
    cimg_library::CImg<float> image;
    cimg_library::CImgDisplay display;
    cimg_library::CImgDisplay hist_display;
    size_t addRay_counter = 0;
    std::vector<std::vector<size_t>> value_counter;
    SimpleCamera camera;
    cimg_library::CImg<float> processed;
    void updateDisplay();
    float bloom_cutoff = 5.0f;
public:
    Gui(const Camera& _camera);
    void work();
    virtual void addRay(float x, float y, float value) override;
    virtual std::pair<glm::vec3, glm::vec3> sampleRay(float x, float y) const override;
    void resetImage();
    // Show all pending pixels
    void finalize();
    void save(const char* path);
private:
    std::mutex ray_mutex;
    void draw_text_overlay(cimg_library::CImg<float>& arg);
};

#endif // GUI_H
