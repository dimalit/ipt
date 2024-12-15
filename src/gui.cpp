#include "gui.h"

#include <CImg.h>

#include <limits>

using namespace std;
using namespace cimg_library;

Gui::Gui()
    :image(640, 640),
     display(image, "IPT", 2),
     hist_display(400,300,"histogram")
{
    value_counter.reserve(640);
    for(size_t i=0; i<640; ++i)
        value_counter.push_back(vector<size_t>(640));
}

void Gui::draw_text_overlay(CImg<float>& arg){
    int mx = display.mouse_x();
    int my = display.mouse_y();
    if(mx<0 || my<0)
        return;
    float white = arg.max();
    arg.draw_text(0,0,"%d %d %e", &white, 0, 1, 13, mx, my, (double)image(mx, my));
}

void Gui::work(){

    int mx_prev = -1, my_prev=-1;

    while(!display.is_closed()){

        int mx = display.mouse_x();
        int my = display.mouse_y();

        if(mx!=mx_prev || my!=my_prev){
            CImg<float> img4text = image;
            draw_text_overlay(img4text);
            display.display(normalize(img4text));
        }// mouse moved

        mx_prev = mx;
        my_prev = my;

        display.wait();
    }// while
}

CImg<float> Gui::normalize(const CImg<float>& arg){
    float inv_gamma = 2.2;
    float max = arg.max(); // arg.kth_smallest(arg.width()*arg.height()*0.95);
    CImg<float> res = (arg/max).pow(1.0f/inv_gamma).cut(0.0f, 1.0f);
    return res;
}

// TODO Too complicated!
void Gui::addRay(float x, float y, float value){
    lock_guard<mutex> lock(ray_mutex);

    size_t ix = x*image.width();
    size_t iy = image.height()-y*image.height();

    ix = min(ix, 640UL-1UL);
    iy = min(iy, 640UL-1UL);

    float old_v = image(ix, iy);
    float sum = old_v*value_counter[iy][ix]+value;
    ++value_counter[iy][ix];
    float new_v = sum/value_counter[iy][ix];
    image(ix, iy) = new_v;

    if(++addRay_counter%500000==0){
        CImg<float> img4text = image;
        draw_text_overlay(img4text);
        display.display(normalize(img4text));
        float max = image.max(); // image.kth_smallest(image.width()*image.height()*0.95);
        float color = 1.0f;
        int n_buckets = 400;
        CImg<float> hist = (+image).histogram(n_buckets, 0.0f, max);
        CImg<float>(400,300).fill(0).draw_graph(hist, &color).display(hist_display);
        //cout << "max=" << max << endl;
    }
}

void Gui::finalize(){
    display.display(normalize(image));
}

void Gui::save(const char *path){
    normalize(image).normalize(0,255).save(path);
}

int old_main(){
    Gui gui;
    gui.work();
    return 0;
}
