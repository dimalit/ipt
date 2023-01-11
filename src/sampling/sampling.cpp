#include "sampling.h"

#include "randf.h"

#include <functional>
#include <cassert>
#include <algorithm>

using namespace glm;

bool p_hit(float prob){
    return randf() < prob;
}

UpperHalfDdf::UpperHalfDdf(){
    max_value = 1.0f;
    full_theoretical_weight = 2.0f*M_PI; // simulate as though pdf=1
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

CosineDdf::CosineDdf(float w){
    max_value = 1.0f/M_PI;
    full_theoretical_weight = w;
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
    return 1.0f/M_PI*arg.z;
}

namespace detail {

class SuperpositionDdf: public DdfImpl {

private:
    std::shared_ptr<const DdfImpl> source, dest;
    static float magic_compute_max_pdf(const DdfImpl* source, const DdfImpl* dest);

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
    UnionDdf(){
        full_theoretical_weight = 0.0f;
    }
    // weight eqals to sum of weights
    virtual glm::vec3 trySample() const override;
};

SuperpositionDdf::SuperpositionDdf(std::shared_ptr<const Ddf> _source, std::shared_ptr<const Ddf> _dest) {

    source = std::dynamic_pointer_cast<const DdfImpl>(_source);
    dest   = std::dynamic_pointer_cast<const DdfImpl>(_dest);

    assert(source && dest);
    assert(!source->isSingular() || !dest->isSingular());
    // this limits applying SuperpositionDdf recursively!
    assert(!isnan(dest->max_value));

    max_value = std::numeric_limits<float>::quiet_NaN(); //magic_compute_max_pdf(source.get(), dest.get());

    full_theoretical_weight = source->full_theoretical_weight * dest->full_theoretical_weight;

    // total_area = source_integral * k; (k=dest->max_pdf)
    // hit_area = this_area
    // hit_rate = hit_area / total_area;
    // adjust result to have result = this_area / 1.0
    full_theoretical_weight *= dest->max_value;    // and mult by source_integal, but this is implicit
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
    float r = randf()*full_theoretical_weight;
    float acc = 0.0f;
    // TODO What's best used here as i?
    for( const std::shared_ptr<const Ddf>& c: components ){
        acc += c->full_theoretical_weight;
        if(r<acc){
            res = c->trySample();
            break;
        }
    }// for
    assert(r<acc);
    return res;
}

}//namespace detail

std::shared_ptr<Ddf> unite(std::shared_ptr<const Ddf> a, std::shared_ptr<const Ddf> b){

    using namespace ::detail;

    if(a==nullptr)
        return unite(std::make_shared<UnionDdf>(), b);
    else if(b==nullptr)
        return unite(std::make_shared<UnionDdf>(), a);

    const UnionDdf* ua = dynamic_cast<const UnionDdf*>(a.get());
    const UnionDdf* ub = dynamic_cast<const UnionDdf*>(b.get());

    if(!ua && ub)
        return unite(b, a);
    if(ua){
        if(ub){
            // add 2 arrays
            std::shared_ptr<UnionDdf> res = std::make_shared<UnionDdf>(*ua);  // copy
            std::copy(ub->components.begin(), ub->components.end(), res->components.end());
            res->full_theoretical_weight += b->full_theoretical_weight;
            return res;
        }
        else{
            // add b to array a
            std::shared_ptr<UnionDdf> res = std::make_shared<UnionDdf>(*ua);  // copy
            if(b->full_theoretical_weight == 0.0f)
                return res;
            res->components.push_back(b);
            res->full_theoretical_weight += b->full_theoretical_weight;
            return res;
        }
    }
    else{
        // add two simple distributions
        std::shared_ptr<UnionDdf> res = std::make_shared<UnionDdf>();
        if(a->full_theoretical_weight != 0.0f)
            res->components.push_back(a);
        if(b->full_theoretical_weight != 0.0f)
            res->components.push_back(b);
        res->full_theoretical_weight = a->full_theoretical_weight + b->full_theoretical_weight;
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
            for( std::shared_ptr<const Ddf> c: u_source->components ){
                std::shared_ptr<const Ddf> sup = chain(c, dest);
                res = unite(res, sup);
            }// for
            return res;
        }
        else{
            // apply b to every a
            std::shared_ptr<UnionDdf> res = std::make_shared<UnionDdf>();
            for( std::shared_ptr<const Ddf> c: u_source->components ){
                std::shared_ptr<SuperpositionDdf> sup = std::make_shared<SuperpositionDdf>(c, dest);
                res->components.push_back( sup );
                res->full_theoretical_weight += sup->full_theoretical_weight;
            }// for
            return res;
        }
    }
    else if( u_dest ){
        // apply a to every b
        std::shared_ptr<UnionDdf> res = std::make_shared<UnionDdf>();
        for( std::shared_ptr<const Ddf> c: u_dest->components ){
            std::shared_ptr<SuperpositionDdf> sup = std::make_shared<SuperpositionDdf>(source, c);
            res->components.push_back( sup );
            res->full_theoretical_weight += sup->full_theoretical_weight;
        }// for
        return res;
    }
    else{
        std::shared_ptr<Ddf> res = std::make_shared<SuperpositionDdf>(source, dest);
        if(res->full_theoretical_weight == 0.0f)
            return nullptr;
        return res;
    }
}

// TODO Add free functions with dynamic_cast for classes above!
