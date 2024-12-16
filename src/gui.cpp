#include "pch.h"

#include "gui.h"

#include <glm/ext/matrix_transform.hpp>

using namespace std;
using namespace glm;
using namespace cimg_library;

static CImg<float> normalize(const CImg<float>& arg){
    float inv_gamma = 2.2;
    float max = arg.max(); // arg.kth_smallest(arg.width()*arg.height()*0.95);
    CImg<float> res = (arg/max).pow(1.0f/inv_gamma).cut(0.0f, 1.0f);
    return res;
}

static void add_patch(CImg<float>& base, const CImg<float>& patch, int cx, int cy){
    int x0 = cx - patch.width()/2;
    int y0 = cy - patch.height()/2;
    for(int y=0; y<patch.height(); ++y){
        for(int x=0; x<patch.width(); ++x){
            base.atXY(x0+x, y0+y) += patch(x,y);
        }// x
    }// y
}

static void draw_halo(CImg<float>& img, int cx, int cy, float C, float r0){
    for(int y=0; y<img.height(); ++y){
        for(int x=0; x<img.width(); ++x){
            float r = hypot(x-cx, y-cy);
            float val = C/(r0+r)/(r0+r);
            img(x, y) += val;
        }// x
    }// y
}

static CImg<float> bloom(const CImg<float>& img, float cutoff){
    CImg<float> out(img, false);
    for(int y=0; y<img.height(); ++y){
        for(int x=0; x<img.width(); ++x){
            float val = img.atXY(x,y);
            if(val <= cutoff)
                continue;

            float radius = 0.5f/3.0f*sqrt(val/cutoff);
            // CImg<float> patch(radius*10, radius*10);
            // patch.draw_gaussian(patch.width()/2,patch.height()/2,CImg<float>::diagonal(radius, radius),&cutoff);
            // add_patch(out, patch, x, y);
            //out.draw_image(0,0,patch);

            draw_halo(out, x, y, cutoff, radius);
        }// x
    }// y
    out.cut(0, cutoff);
    return out;
}

Gui::Gui(const Camera& _camera)
    :image(640, 640),
     display(image, "IPT", 2),
     hist_display(400,300,"histogram"),
     camera(dynamic_cast<const SimpleCamera*>(&_camera)->position, dynamic_cast<const SimpleCamera*>(&_camera)->direction),
     processed(image)
{
    value_counter.reserve(640);
    for(size_t i=0; i<640; ++i)
        value_counter.push_back(vector<size_t>(640));
    display.set_wheel();
}

void Gui::draw_text_overlay(CImg<float>& arg){
    int mx = display.mouse_x();
    int my = display.mouse_y();
    if(mx<0 || my<0)
        return;
    float white = arg.max();
    arg.draw_text(0,0,"%d %d %e", &white, 0, 1, 13, mx, my, (double)image(mx, my));
}

void Gui::updateDisplay(){
    processed = bloom(image,this->bloom_cutoff);
    display.display(normalize(processed));

    CImg<float> img4hist = processed;
    float max = img4hist.max();
    float color = 1.0f;
    int n_buckets = 400;
    CImg<float> hist = img4hist.histogram(n_buckets, max/100000.0f, max);
    CImg<float>(400,300).fill(0).draw_graph(hist, &color).display(hist_display);
    //cout << "max=" << max << endl;
}

void Gui::work(){

    int mx_prev = -1, my_prev=-1;

    while(!display.is_closed()){

        int mx = display.mouse_x();
        int my = display.mouse_y();

        if(mx!=mx_prev || my!=my_prev){
            CImg<float> img4text = +processed;
            draw_text_overlay(img4text);
            display.display(normalize(img4text));
        }// mouse moved

        if(display.is_key()){

            bool need_camera_update = false;

            if(display.key()==cimg::keyARROWLEFT){
                mat3 mat = glm::rotate(glm::identity<mat4>(), (float)-M_PI/12, vec3(0,0,1));
                camera.position = mat * camera.position;
                camera.direction = mat*camera.direction;
                need_camera_update = true;
            }
            else if(display.key()==cimg::keyARROWRIGHT){
                mat3 mat = glm::rotate(glm::identity<mat4>(), (float)+M_PI/12, vec3(0,0,1));
                camera.position = mat * camera.position;
                camera.direction = mat*camera.direction;
                need_camera_update = true;
            }
            else if(display.key()==cimg::keyARROWDOWN){
                camera.position *= 1.1;
                need_camera_update = true;
            }
            else if(display.key()==cimg::keyARROWUP){
                camera.position /= 1.1;
                need_camera_update = true;
            }

            if(need_camera_update){
                camera.right = glm::normalize(glm::cross(camera.direction, vec3(0,0,1)));
                camera.up = glm::normalize(glm::cross(camera.right, camera.direction));
                resetImage();
                updateDisplay();
            }

        }// if key

        if(display.wheel()){
            this->bloom_cutoff *= pow(sqrt(2.0f), display.wheel());
            updateDisplay();
            display.set_wheel();
        }

        mx_prev = mx;
        my_prev = my;

        display.wait();
    }// while
}

void Gui::resetImage(){
    lock_guard<mutex> lock(ray_mutex);
    image.fill(0.0f);
    for(int y=0; y<image.height(); ++y){
        for(int x=0; x<image.width(); ++x){
            value_counter[y][x] = 0.0f;
        }// x
    }// y
}

// TODO Too complicated!
void Gui::addRay(float x, float y, float value){
    lock_guard<mutex> lock(ray_mutex);

    size_t ix = x*image.width();
    size_t iy = image.height()-y*image.height();

    ix = std::min(ix, 640UL-1UL);
    iy = std::min(iy, 640UL-1UL);

    float old_v = image(ix, iy);
    float sum = old_v*value_counter[iy][ix]+value;
    ++value_counter[iy][ix];
    float new_v = sum/value_counter[iy][ix];
    image(ix, iy) = new_v;

    if(++addRay_counter%500000==0)
        updateDisplay();
}

std::pair<glm::vec3, glm::vec3> Gui::sampleRay(float x, float y) const {
    return camera.sampleRay(x,y);
};

void Gui::finalize(){
    display.display(normalize(image));
}

void Gui::save(const char *path){
    normalize(image).normalize(0,255).save(path);
}

