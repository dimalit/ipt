#include "ddf_detail.h"

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

float alpha_from_i(size_t alpha_bucket, size_t n){
    return (alpha_bucket+0.5f)/n*M_PI;
}

float phi_from_i(size_t phi_bucket, size_t n){
    return (phi_bucket+0.5f)/n*2.0f*M_PI;
}

float bucket_area(size_t alpha_bucket, size_t phi_bucket, size_t alpha_buckets, size_t phi_buckets){
    float alpha = alpha_from_i(alpha_bucket, alpha_buckets);
    float phi   = phi_from_i(phi_bucket, phi_buckets);

    float dy = M_PI / alpha_buckets;
    float dx = 2.0f*M_PI*sin(alpha) / phi_buckets;

    return dx*dy;
}

// TODO does not work for sphere, as it gives wrong area
// (only visible)
void mc_integral_and_max(const Ddf& ddf, float& ddf_integral, float& ddf_max){

    SphericalDdf sph;

    ddf_integral = 0.0f;
    ddf_max = 0.0f;
    for(size_t i = 0; i < N; ++i){

        vec3 dir = vec3();
        while(dir == vec3())
            dir = normalize(sph.sample());

        float value = ddf.value(dir);
        if(value > ddf_max)
            ddf_max = value;
        ddf_integral += value/sph.value(dir);

    }// for

    ddf_integral /= N;
}

void rects_integral_and_max(const Ddf& ddf, float& ddf_integral, float& ddf_max){
    for(size_t i=0; i<20; ++i){
        for(size_t j=0; j<20; ++j){
            float alpha = alpha_from_i(i, 20);
            float phi   = phi_from_i(j, 20);
            float value = ddf.value(polar2vec(alpha, phi));
            if(value > ddf_max)
                ddf_max = value;
            float area = bucket_area(i, j, 20, 20);
            ddf_integral += value*area;
        }
    }
}

int normalize_phi(size_t j, size_t n){
    if(j<0)
        j+=n;
    if(j>=n)
        j-=n;
    return j;
}

bool has_0_neighbours(const Ddf& ddf, size_t i, size_t j, float* buckets, size_t size_alpha, size_t size_phi){
    int ii, jj;
    float value;

    ii=i+1;
    jj=j;
    value = ddf.value(polar2vec(alpha_from_i(ii, size_alpha), phi_from_i(jj, size_phi)));
    if(ii<size_alpha && (buckets[ii*size_phi+jj]==0.0f || value==0.0f))
        return true;

    ii=i-1;
    jj=j;
    value = ddf.value(polar2vec(alpha_from_i(ii, size_alpha), phi_from_i(jj, size_phi)));
    if(ii>=0 && (buckets[ii*size_phi+jj]==0.0f || value==0.0f))
        return true;

    ii=i;
    jj=normalize_phi(j+1, size_phi);
    value = ddf.value(polar2vec(alpha_from_i(ii, size_alpha), phi_from_i(jj, size_phi)));
    if(buckets[ii*size_phi+jj]==0.0f || value==0.0f)
        return true;

    ii=i;
    jj=normalize_phi(j-1, size_phi);
    value = ddf.value(polar2vec(alpha_from_i(ii, size_alpha), phi_from_i(jj, size_phi)));
    if(buckets[ii*size_phi+jj]==0.0f || value==0.0f)
        return true;

    return false;
}

bool check_ddf(const Ddf& ddf, bool strict_integral, size_t size_alpha, size_t size_phi){
    // 1 compute ddf integral and max

    float ddf_integral;
    float ddf_max;

    mc_integral_and_max(ddf, ddf_integral, ddf_max);

    float buckets[size_alpha][size_phi];
    memset(&buckets[0][0], 0, sizeof(buckets));
    size_t total_tries = 0;

    // 2 test chi-square

    // 2.1 fill buckets
    for(size_t i=0; i<N; ++i){

        ++total_tries;
        vec3 vec = normalize(ddf.sample());
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

        size_t alpha_bucket = alpha/M_PI * size_alpha;
        size_t phi_bucket   = phi/2.0/M_PI * size_phi;

        ++buckets[alpha_bucket][phi_bucket];
    }// for

    // 2.2 compare buckets against theory
    size_t exp_counter = 0;
    float theor_counter = 0.0f;
    float chi2 = 0.0f;
    size_t dof_skip_counter = 0; // for chi^2

    for(size_t i=0; i<size_alpha; ++i){
        for(size_t j=0; j<size_phi; ++j){
            float theor = ddf.value(polar2vec(alpha_from_i(i, size_alpha), phi_from_i(j, size_phi)))*bucket_area(i, j, size_alpha, size_phi)*N / ddf_integral;
            float exper = buckets[i][j];

            exp_counter += buckets[i][j];
            theor_counter += theor;

            // TODO We ignore partially-iverlapping buckets! Look for paper about "chi-square test for discontinuous distributions"
            bool use_it = theor > 2 && exper >= 2 && ! has_0_neighbours(ddf, i,j,&buckets[0][0],size_alpha,size_phi);
            if(use_it)
                chi2 += pow(exper-theor, 2) / theor;
            else
                ++dof_skip_counter;

//            cout << i << "\t" << j << "\t" << exper << "\t"
//                 << theor << " -> " << (theor > 1e-6 ? exper/theor : -1) << " " << (use_it?"use":"skip") << endl;

        }// for j
    }// for i

    cout << "\tDDF integral = " << ddf_integral << endl;

    cout << "\tExperimental Total = " << exp_counter << endl;
    cout << "\tTheoretical total  = " << theor_counter << endl;
    float success_ratio = (float)N/total_tries;
    cout << "\tSample success ratio = " << success_ratio << endl;

    size_t total_buckets = size_alpha * size_phi;
    float chi_floor = 70, chi_ceil = 135;       // for 100
    chi_floor *= (total_buckets-dof_skip_counter) / 100.0f;
    chi_ceil *= (total_buckets-dof_skip_counter) / 100.0f;
    cout << "\tChi^2 " << total_buckets-dof_skip_counter << " DoF (" << chi_floor << "-" << chi_ceil << ") = " << chi2 << endl;

    return chi2 > chi_floor && chi2 < chi_ceil &&
           success_ratio/ddf_integral > 0.95f && success_ratio/ddf_integral < 1.05f &&
           (!strict_integral || ddf_integral > 0.95f && ddf_integral < 1.05f);
}

