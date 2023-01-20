#include "sampling.h"

#include "randf.h"

#include <functional>
#include <cassert>
#include <algorithm>

using namespace glm;
using namespace std;

bool p_hit(float prob){
    return randf() < prob;
}

SphericalDdf::SphericalDdf(){
    max_value = 0.25/M_PI;
}
vec3 SphericalDdf::trySample() const {
    float u1 = randf()*2.0f - 1.0f;
    float u2 = randf();
    float alpha = acos(u1);
    float phi = 2*M_PI*u2;
    float r = sin(alpha);
    return vec3(r*cos(phi), r*sin(phi), u1);
}
float SphericalDdf::value( vec3 ) const {
    // TODO assert length = 1?
    return max_value;
}

UpperHalfDdf::UpperHalfDdf(){
    max_value = 1.0f;
}
vec3 UpperHalfDdf::trySample() const {
    float u1 = randf();
    float u2 = randf();
    float alpha = acos(u1);
    float phi = 2*M_PI*u2;
    float r = sin(alpha);
    return vec3(r*cos(phi), r*sin(phi), u1);
}
float UpperHalfDdf::value( vec3 arg ) const {
    // TODO assert length = 1?
    // TODO function for this like clip()?
    if(arg.z < 0.0f)
        return 0.0f;
    return max_value;
}

CosineDdf::CosineDdf(){
    max_value = 1.0f/M_PI;
}
vec3 CosineDdf::trySample() const {
    float u1 = randf();
    float u2 = randf();
    float alpha = acos(sqrt(u1));
    float phi = 2*M_PI*u2;
    float r = sin(alpha);
    return vec3(r*cos(phi), r*sin(phi), cos(alpha));
}
float CosineDdf::value( vec3 arg ) const {
    // TODO assert length = 1?
    // TODO function for this like clip()?
    if(arg.z < 0.0f)
        return 0.0f;
    return arg.z;
}

namespace detail {

class SuperpositionDdf: public Ddf {

private:
    std::shared_ptr<const Ddf> source, dest;
    static float magic_compute_max_pdf(const Ddf* source, const Ddf* dest);

public:

    SuperpositionDdf(std::shared_ptr<const Ddf> _source, std::shared_ptr<const Ddf> _dest);

    virtual float value( glm::vec3 x ) const override {
        return source->value(x) * dest->value(x);
    }

    virtual glm::vec3 trySample() const override;
};

// TODO Indicate somehow that trySample should always succeed!
struct UnionDdf: public Ddf {
    std::vector< std::shared_ptr<const Ddf> > components;
    std::vector< float > weights;
    UnionDdf(){
    }
    // weight eqals to sum of weights
    virtual glm::vec3 trySample() const override;
    virtual float value( vec3 arg ) const override;
};

SuperpositionDdf::SuperpositionDdf(std::shared_ptr<const Ddf> _source, std::shared_ptr<const Ddf> _dest) {

    source = std::dynamic_pointer_cast<const Ddf>(_source);
    dest   = std::dynamic_pointer_cast<const Ddf>(_dest);

    assert(source && dest);
    assert(!source->isSingular() || !dest->isSingular());
    // this limits applying SuperpositionDdf recursively!
    assert(!isnan(dest->max_value));

    max_value = std::numeric_limits<float>::quiet_NaN(); //magic_compute_max_pdf(source.get(), dest.get());

    // total_area = source_integral * k; (k=dest->max_pdf)
    // hit_area = this_area
    // hit_rate = hit_area / total_area;
    // adjust result to have result = this_area / 1.0
    //full_theoretical_weight *= dest->max_value;    // and mult by source_integal, but this is implicit
}

vec3 SuperpositionDdf::trySample() const {

    vec3 x = source->trySample();
    if(x == vec3())         // fail if fail
        return x;

    // NB should not be both singular
    if(source->isSingular())
        return x;

    if(dest->isSingular())
        return dest->trySample();

    // As we need source*dest <= source*k,
    // we get k >= dest
    float k = dest->max_value;

    bool hit = p_hit( this->value( x ) / (source->value( x )*k) );

    return hit ? x : vec3();
}

// As from different points Union distribution looks differently -
// it would fail with different rate for different points,
// thus modeling difference in lighting
vec3 UnionDdf::trySample() const {
    vec3 res;
    if(components.size() == 0)
        return vec3();
    float r = randf();
    float acc = 0.0f;
    // TODO What's best used here as i?
    for(size_t i=0; i<components.size(); ++i){
        acc += weights[i];
        if(r<acc){
            res = components[i]->trySample();
            break;
        }
    }// for
    assert(r<acc);
    return res;
}

float UnionDdf::value( vec3 arg ) const {
    float res = 0.0f;
    for(size_t i=0; i<components.size(); ++i){
        res += weights[i] * components[i]->value(arg);
    }// for
    return res;
}


}//namespace detail

std::shared_ptr<Ddf> unite(shared_ptr<const Ddf> a, float ka, shared_ptr<const Ddf> b, float kb){

    using namespace ::detail;

    if(a==nullptr)
        return unite(make_shared<UnionDdf>(), 1.0f, b, 1.0f);
    else if(b==nullptr)
        return unite(make_shared<UnionDdf>(), 1.0f, a, 1.0f);

    const UnionDdf* ua = dynamic_cast<const UnionDdf*>(a.get());
    const UnionDdf* ub = dynamic_cast<const UnionDdf*>(b.get());

    if(!ua && ub)
        return unite(b, kb, a, ka);
    if(ua){
        if(ub){
            // add 2 arrays
            std::shared_ptr<UnionDdf> res = std::make_shared<UnionDdf>();  // empty

            // copy a components and weights
            std::copy(ua->components.begin(), ua->components.end(), res->components.end());
            std::for_each(ua->weights.begin(), ua->weights.end(), [&res,ka,kb](float k){
                res->weights.push_back(k*ka/(ka+kb));
            });
            // copy b components and weights
            std::copy(ub->components.begin(), ub->components.end(), res->components.end());
            std::for_each(ub->weights.begin(), ub->weights.end(), [&res,ka,kb](float k){
                res->weights.push_back(k*kb/(ka+kb));
            });
            return res;
        }
        else{
            // add b to array a
            std::shared_ptr<UnionDdf> res = std::make_shared<UnionDdf>(*ua);  // copy
            // re-weigh
            std::for_each(res->weights.begin(), res->weights.end(), [ka,kb](float& k){
                k*=ka/(ka+kb);
            });
            res->components.push_back(b);
            res->weights.push_back(kb/(ka+kb));
            return res;
        }
    }
    else{
        // add two simple distributions
        std::shared_ptr<UnionDdf> res = std::make_shared<UnionDdf>();
        res->components.push_back(a);
        res->weights.push_back(ka/(ka+kb));
        res->components.push_back(b);
        res->weights.push_back(kb/(ka+kb));
        return res;
    }
}

// NB res and args can be null!
std::shared_ptr<const Ddf> chain(std::shared_ptr<const Ddf> source, std::shared_ptr<const Ddf> dest){

    using namespace ::detail;

    if(!source)
        return dest;
    if(!dest)
        return source;

    const UnionDdf* u_source = dynamic_cast<const UnionDdf*>(source.get());
    const UnionDdf* u_dest = dynamic_cast<const UnionDdf*>(dest.get());

    if( u_source ){
        if( u_dest ){
            // make cross-product
            // TODO do this algorithm nicer?!
            std::shared_ptr<const Ddf> res = std::make_shared<UnionDdf>();
            // TODO normal for with i
            size_t i=0;
            for( std::shared_ptr<const Ddf> c: u_source->components ){
                std::shared_ptr<const Ddf> sup = chain(c, dest);
                res = unite(res, i, sup, 1.0f);
            }// for
            return res;
        }
        else{
            // apply b to every a
            std::shared_ptr<UnionDdf> res = std::make_shared<UnionDdf>();
            // TODO normal for with i
            size_t i=0;
            for( std::shared_ptr<const Ddf> c: u_source->components ){
                std::shared_ptr<SuperpositionDdf> sup = std::make_shared<SuperpositionDdf>(c, dest);
                res->components.push_back( sup );
                res->weights.push_back(u_source->weights[i++]);
            }// for
            return res;
        }
    }
    else if( u_dest ){
        // apply a to every b
        std::shared_ptr<UnionDdf> res = std::make_shared<UnionDdf>();
        // TODO normal for with i
        size_t i=0;
        for( std::shared_ptr<const Ddf> c: u_dest->components ){
            std::shared_ptr<SuperpositionDdf> sup = std::make_shared<SuperpositionDdf>(source, c);
            res->components.push_back( sup );
            res->weights.push_back(u_dest->weights[i++]);
        }// for
        return res;
    }
    else{
        std::shared_ptr<Ddf> res = std::make_shared<SuperpositionDdf>(source, dest);
        return res;
    }
}

// TODO Add free functions with dynamic_cast for classes above!
