#include "sampling_detail.h"

#include <functional>
#include <cassert>
#include <algorithm>
#include <random>

using namespace glm;

// TODO deduplicate
static struct{
    std::random_device rdev;
    std::mt19937 gen = std::mt19937(rdev());

    std::uniform_real_distribution<> dist = std::uniform_real_distribution<>(0.0f, 1.0f);

    float operator()() {
        return dist(gen);
    }
} randf;

bool p_hit(float prob){
    return randf() < prob;
}

UpperHalf::UpperHalf(){
    max_pdf = 0.5f/M_PI;
    weight = 2.0f*M_PI; // simulate as though pdf=1
}
vec3 UpperHalf::trySample() const {
    float u1 = randf();
    float u2 = randf();
    float alpha = acos(u1);
    float phi = 2*M_PI*u2;
    float r = sin(alpha);
    return vec3(r*cos(phi), r*sin(phi), u1);
}
float UpperHalf::pdf( vec3 arg ) const {
    // TODO assert length = 1?
    // TODO function for this like clip()?
    if(arg.z < 0.0f)
        return 0.0f;
    return max_pdf;
}

Cosine::Cosine(float w){
    max_pdf = 1.0f/M_PI;
    weight = w;
}
vec3 Cosine::trySample() const {
    float u1 = randf();
    float u2 = randf();
    float alpha = acos(sqrt(u1));
    float phi = 2*M_PI*u2;
    float r = sin(alpha);
    return vec3(r*cos(phi), r*sin(phi), cos(alpha));
}
float Cosine::pdf( vec3 arg ) const {
    // TODO assert length = 1?
    // TODO function for this like clip()?
    if(arg.z < 0.0f)
        return 0.0f;
    return 1.0f/M_PI*arg.z;
}

Superposition::Superposition(std::shared_ptr<const Distribution> _source, std::shared_ptr<const Distribution> _dest) {

    source = std::dynamic_pointer_cast<const DistributionImpl>(_source);
    dest   = std::dynamic_pointer_cast<const DistributionImpl>(_dest);

    assert(source && dest);
    // We cannot apply Superposition recursively!
    // We need source's pdf integral in trySample()
    assert(dynamic_cast<const Superposition*>(_source.get()) == nullptr);

    max_pdf = magic_compute_max_pdf(source.get(), dest.get());

    weight = source->weight * dest->weight;

    // total_area = 1 * k; (k=max_pdf)
    // hit_area = this_area
    // hit_rate = hit_area / total_area;
    // adjust result to have result = this_area / 1.0
    weight *= dest->max_pdf;
}

vec3 Superposition::trySample() const {

    vec3 x = source->trySample();
    assert(x != vec3());            // should always succeed because source->pdf_integrates_to_one

    // NB should not be both singular
    if(source->isSingular())
        return x;

    if(dest->isSingular())
        return dest->trySample();

    // As we need source*dest <= source*k,
    // we get k >= dest
    float k = dest->max_pdf;

    bool hit = p_hit( this->pdf( x ) / (source->pdf( x )*k) );

    return hit ? x : vec3();
}

vec3 Union::trySample() const {
    vec3 res;
    do{
        float r = randf()*weight;
        float acc = 0.0f;
        // TODO What's best used here as i?
        for( const std::shared_ptr<const Distribution>& c: components ){
            acc += c->weight;
            if(r<acc)
                res = c->trySample();
        }// for
        assert(false && "Loop should always break insise!");
    }while(res == vec3());
    return res;
}

std::shared_ptr<Distribution> unite(std::shared_ptr<const Distribution> a, std::shared_ptr<const Distribution> b){

    const Union* ua = dynamic_cast<const Union*>(a.get());
    const Union* ub = dynamic_cast<const Union*>(b.get());

    if(!ua && ub)
        return unite(b, a);
    if(ua){
        if(ub){
            // add 2 arrays
            std::shared_ptr<Union> res = std::make_shared<Union>(*ua);  // copy
            std::copy(ub->components.begin(), ub->components.end(), res->components.end());
            res->weight += b->weight;
            return res;
        }
        else{
            // add b to array a
            std::shared_ptr<Union> res = std::make_shared<Union>(*ua);  // copy
            res->components.push_back(b);
            res->weight += a->weight;
            return res;
        }
    }
    else{
        // add two simple distributions
        std::shared_ptr<Union> res = std::make_shared<Union>();
        res->components.push_back(a);
        res->components.push_back(b);
        res->weight = a->weight + b->weight;
        return res;
    }
}

// NB res and args can be null!
std::shared_ptr<const Distribution> apply(std::shared_ptr<const Distribution> source, std::shared_ptr<const Distribution> dest){

    if(!source)
        return dest;
    if(!dest)
        return source;

    const Union* u_source = dynamic_cast<const Union*>(source.get());
    const Union* u_dest = dynamic_cast<const Union*>(dest.get());

    if( u_source ){
        if( u_dest ){
            // make cross-product
            // TODO do this algorithm nicer?!
            std::shared_ptr<const Distribution> res = std::make_shared<Union>();
            for( std::shared_ptr<const Distribution> c: u_source->components ){
                std::shared_ptr<const Distribution> sup = apply(c, dest);
                res = unite(res, sup);
            }// for
            return res;
        }
        else{
            // apply b to every a
            std::shared_ptr<Union> res = std::make_shared<Union>();
            for( std::shared_ptr<const Distribution> c: u_source->components ){
                std::shared_ptr<Superposition> sup = std::make_shared<Superposition>(c, dest);
                res->components.push_back( sup );
                res->weight += sup->weight;
            }// for
            return res;
        }
    }
    else if( u_dest ){
        // apply a to every b
        std::shared_ptr<Union> res = std::make_shared<Union>();
        for( std::shared_ptr<const Distribution> c: u_dest->components ){
            std::shared_ptr<Superposition> sup = std::make_shared<Superposition>(source, c);
            res->components.push_back( sup );
            res->weight += sup->weight;
        }// for
        return res;
    }
    else{
        std::shared_ptr<Distribution> res = std::make_shared<Superposition>(source, dest);
        if(res->weight == 0.0f)
            return nullptr;
        return res;
    }
}

// TODO Add free functions with dynamic_cast for classes above!
