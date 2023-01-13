#include <sampling/sampling.h>

#include <iostream>

#include <cstring>

using namespace glm;
using namespace std;

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

bool check_ddf(const Ddf& ddf){
    const ::detail::DdfImpl* ddi = dynamic_cast<const ::detail::DdfImpl*>(&ddf);

    // 1 compute ddf integral and max

    float ddf_integral = 0.0f;
    float ddf_max = 0.0f;
    for(size_t i=0; i<20; ++i){
        for(size_t j=0; j<20; ++j){
            float alpha = alpha_from_i(i);
            float phi   = phi_from_i(j);
            float value = ddi->value(polar2vec(alpha, phi));
            if(value > ddf_max)
                ddf_max = value;
            float area = bucket_area(i, j);
            ddf_integral += value*area;
        }
    }

    const size_t N = 100000;
    float buckets[20][20];
    memset(&buckets[0][0], 0, sizeof(buckets));
    size_t total_tries = 0;

    // 2 test chi-square

    // 2.1 fill buckets
    for(size_t i=0; i<N; ++i){

        ++total_tries;
        vec3 vec = ddi->trySample();
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
            float theor = ddi->value(polar2vec(alpha, phi))*bucket_area(i,j)*N/ddf_integral;
            float exper = buckets[i][j];

//            cout << i << "\t" << j << "\t" << exper/bucket_area(i,j) << "\t"
//                 << theor/bucket_area(i,j) << " -> " << (theor > 1e-6 ? exper/theor : -1) << endl;

            exp_counter += buckets[i][j];
            theor_counter += theor;

            if(theor > 2 && exper >= 2)
                chi2 += pow(exper-theor, 2) / theor;
            else
                ++dof_skip_counter;
        }
    }

    cout << "DDF integral = " << ddf_integral << endl;
    cout << "DDF max      = " << ddf_max << " / " << ddi->max_value << endl;

    cout << "Experimental Total = " << exp_counter << endl;
    cout << "Theoretical total  = " << theor_counter << endl;
    cout << "Total tries        = " << total_tries << endl;

    float chi_floor = 70, chi_ceil = 135;       // for 100
    chi_floor *= (400.0f-dof_skip_counter) / 100;
    chi_ceil *= (400.0f-dof_skip_counter) / 100;
    cout << "Chi^2 " << 400-dof_skip_counter << " DoF (" << chi_floor << "-" << chi_ceil << ") = " << chi2 << endl;

    return chi2 > chi_floor && chi2 < chi_ceil &&
           (isnan(ddi->max_value) || ddf_max/ddi->max_value > 0.9 && ddf_max/ddi->max_value < 1.1);
}
