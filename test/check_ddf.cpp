#include <libddf/ddf_detail.h>

#include <iostream>

#include <cstring>

using namespace glm;
using namespace std;

// TODO This whole file is duplicated. It's better then duplicate just lines of code, but maybe do something with it.

const size_t N = 100000;

vec3 polar2vec(float alpha, float phi){
    float r = sin(alpha);
    return vec3(r*cos(phi), r*sin(phi), cos(alpha));
}

float alpha_from_i(size_t alpha_bucket){
    return (alpha_bucket+0.5f)/20.0f*M_PI;
}

float phi_from_i(size_t phi_bucket){
    return (phi_bucket+0.5f)/20.0f*2.0f*M_PI;
}

float bucket_area(size_t alpha_bucket, size_t phi_bucket){
    float alpha = alpha_from_i(alpha_bucket);
    float phi   = phi_from_i(phi_bucket);

    float dy = M_PI / 20.0f;
    float dx = 2.0f*M_PI*sin(alpha) / 20.0f;

    return dx*dy;
}

void mc_integral_and_max(Ddf& ddf, float& ddf_integral, float& ddf_max){

    SphericalDdf sph;

    ddf_integral = 0.0f;
    ddf_max = 0.0f;
    for(size_t i = 0; i < N; ++i){

        vec3 dir = vec3();
        while(dir == vec3())
            dir = sph.trySample();

        float value = ddf.value(dir);
        if(value > ddf_max)
            ddf_max = value;
        ddf_integral += value;

    }// for

    ddf_integral *= 4.0f*M_PI/N;
}

void rects_integral_and_max(Ddf& ddf, float& ddf_integral, float& ddf_max){
    for(size_t i=0; i<20; ++i){
        for(size_t j=0; j<20; ++j){
            float alpha = alpha_from_i(i);
            float phi   = phi_from_i(j);
            float value = ddf.value(polar2vec(alpha, phi));
            if(value > ddf_max)
                ddf_max = value;
            float area = bucket_area(i, j);
            ddf_integral += value*area;
        }
    }
}


bool check_ddf(Ddf& ddf){
    // 1 compute ddf integral and max

    float ddf_integral;
    float ddf_max;

    mc_integral_and_max(ddf, ddf_integral, ddf_max);

    float buckets[20][20];
    memset(&buckets[0][0], 0, sizeof(buckets));
    size_t total_tries = 0;

    // 2 test chi-square

    // 2.1 fill buckets
    for(size_t i=0; i<N; ++i){

        ++total_tries;
        vec3 vec = ddf.trySample();
        if(vec == vec3()){
            --i;
            continue;
        }

        float alpha = acos(vec.z);
        float r = length(vec3(vec.x, vec.y, 0.0f));
        float phi;
        if(vec.x >= 0.0f)
            phi = asin(vec.y / r);
        else
            phi = M_PI - asin(vec.y / r);

        if(phi < 0.0f)
            phi += 2.0f*M_PI;
        if(phi >= 2.0f*M_PI)
            phi -= 2.0f*M_PI;

        assert(alpha >= -1e-6 && alpha < M_PI+1e-6);
        assert(phi >= -1e-6 && phi < 2.0f*M_PI+1e-6);

        size_t alpha_bucket = alpha/M_PI * 20.0f;
        size_t phi_bucket   = phi/2.0/M_PI * 20.0f;

        ++buckets[alpha_bucket][phi_bucket];
    }// for

    // 2.2 compare buckets against theory
    size_t exp_counter = 0;
    float theor_counter = 0.0f;
    float chi2 = 0.0f;
    size_t dof_skip_counter = 0; // for chi^2

    for(size_t i=0; i<20; ++i){
        for(size_t j=0; j<20; ++j){
            float alpha = alpha_from_i(i);
            float phi   = phi_from_i(j);
            float theor = ddf.value(polar2vec(alpha, phi))*bucket_area(i,j)*N;// we are trying to make it 1: /ddf_integral;
            float exper = buckets[i][j];

//            cout << i << "\t" << j << "\t" << exper << "\t"
//                 << theor << " -> " << (theor > 1e-6 ? exper/theor : -1) << endl;

            exp_counter += buckets[i][j];
            theor_counter += theor;

            if(theor > 2 && exper >= 2)
                chi2 += pow(exper-theor, 2) / theor;
            else
                ++dof_skip_counter;
        }
    }

    cout << "DDF integral = " << ddf_integral << endl;
    cout << "DDF max      = " << ddf_max << " / " << ddf.max_value << endl;

    cout << "Experimental Total = " << exp_counter << endl;
    cout << "Theoretical total  = " << theor_counter << endl;
    cout << "Total tries        = " << total_tries << endl;

    float chi_floor = 70, chi_ceil = 135;       // for 100
    chi_floor *= (400.0f-dof_skip_counter) / 100;
    chi_ceil *= (400.0f-dof_skip_counter) / 100;
    cout << "Chi^2 " << 400-dof_skip_counter << " DoF (" << chi_floor << "-" << chi_ceil << ") = " << chi2 << endl;

    return chi2 > chi_floor && chi2 < chi_ceil &&
           (isnan(ddf.max_value) || ddf_max/ddf.max_value > 0.9 && ddf_max/ddf.max_value < 1.1);
}
