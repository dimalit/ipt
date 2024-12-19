#include "pch.h"

#include<iostream>

using namespace std;
using namespace cimg_library;

CImg<float> image(640, 640);
CImg<float> processed = image;
CImgDisplay display(processed, "Glare Paint", 2);

static void draw_halo(CImg<float>& img, int cx, int cy, float C, float r0){
    for(int y=0; y<img.height(); ++y){
        for(int x=0; x<img.width(); ++x){
            float r = hypot(x-cx, y-cy);
            float val = C/(r0+r)/(r0+r);
            img(x, y) += val;
        }// x
    }// y
}

static CImg<float> glare(const CImg<float>& img, float cutoff){
    CImg<float> out(img, false);
    for(int y=0; y<img.height(); ++y){
        for(int x=0; x<img.width(); ++x){
            float val = img(x,y);
            if(val <= cutoff)
                continue;

            float coef = 0.1*val/cutoff;
            draw_halo(out, x, y, cutoff*coef, 0.25f);
        }// x
    }// y
    out.cut(0, cutoff);
    return out;
}

static CImg<float> normalize4pixels(const CImg<float>& arg){
    float inv_gamma = 2.2;
    float max = arg.max();
    CImg<float> res = (arg/max).pow(1.0f/inv_gamma).cut(0.0f, 1.0f);
    return res;
}

static void update_display(){
    display.display( normalize4pixels(processed) );
}

int main(){
    while(!display.is_closed()){

        // LMB
        if(display.button() & 1){
            int mx = display.mouse_x();
            int my = display.mouse_y();
            for(int dy=-3; dy <= 3; ++dy)
            for(int dx=-3; dx <= 3; ++dx)
                image.atXY(mx+dx, my+dy) += 1.0f;
            processed = glare(image, 1);
            update_display();
        }// if LMB

        // if(mx!=mx_prev || my!=my_prev){
        //     CImg<float> img4text = +processed;
        //     draw_text_overlay(img4text);
        //     display.display(normalize(img4text));
        // }// mouse moved

        if(display.is_key()){
        }// if key

        if(display.wheel()){
            // this->bloom_cutoff *= pow(sqrt(2.0f), display.wheel());
            // updateDisplay();
            // display.set_wheel();
        }
        display.wait();
    }// while
    return 0;
}
