#include "ddf_detail.h"

#include "randf.h"

#include <boost/pool/pool.hpp>

#include <functional>
#include <cassert>
#include <algorithm>

using namespace glm;
using namespace std;

size_t Ddf::object_counter = 0;

bool p_hit(float prob){
    return randf() < prob;
}

class segregated_allocator {

    static std::map<size_t, std::unique_ptr<boost::pool<>>> pools_map;

public:

    // reserves sizeof(void*) bytes more and stores pointer to pool in that space!
    static void* malloc(size_t size){
        size_t adjusted_size = size + sizeof(boost::pool<>*);
        unique_ptr<boost::pool<>>& ptr = pools_map[size];
        if(ptr==nullptr){
            ptr.reset( new boost::pool<>(adjusted_size) );
            cout << "NEW POOL " << pools_map.size() << " size = " << size << endl;
            assert(pools_map.size() < 100);
        }
        boost::pool<>** memory = (boost::pool<>**)ptr->malloc();
        memory[0] = ptr.get();

        // TODO Comprehensive logs infrastructure for this!
        //cout << "ALLOCATE " << &memory[1] << endl;

        return &memory[1];
    }

    static void free(void* ptr){
        //cout << "DEALLOCATE " << ptr << endl;
        boost::pool<>** pool_ptr = (boost::pool<>**) ptr;
        boost::pool<>** adjusted_ptr = pool_ptr-1;
        adjusted_ptr[0]->free(adjusted_ptr);
    }
};
std::map<size_t, std::unique_ptr<boost::pool<>>> segregated_allocator::pools_map;

// reserves sizeof(void*) bytes more and stores pointer to pool in that space!
void* Ddf::operator new(size_t size){
    return segregated_allocator::malloc(size);
}

void Ddf::operator delete(void* ptr){
    segregated_allocator::free(ptr);
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
    max_value = 0.5f/M_PI;
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
    float cos_alpha = sqrt(u1);
    float alpha = acos(cos_alpha);
    float phi = 2*M_PI*u2;
    float r = sin(alpha);
    return vec3(r*cos(phi), r*sin(phi), cos_alpha);
}
float CosineDdf::value( vec3 arg ) const {
    // TODO assert length = 1?
    // TODO function for this like clip()?
    if(arg.z < 0.0f)
        return 0.0f;
    return arg.z/M_PI;
}

namespace detail {

template<class T>
class segregated_allocator_class {
public:
    using value_type = T;
    T* allocate(size_t count){
        return (T*)segregated_allocator::malloc(count*sizeof(T));
    }
    void deallocate(T* ptr, size_t){
        segregated_allocator::free(ptr);
    }
};

template<class P=Ddf*>
class SuperpositionDdf: public Ddf {

private:
    P source, dest;
    static float magic_compute_max_pdf(Ddf* source, Ddf* dest);

public:

    SuperpositionDdf(P _source, P _dest);

    virtual float value( glm::vec3 x ) const override {
        return source->value(x) * dest->value(x) / dest->max_value;
    }

    virtual glm::vec3 trySample() const override;
};

// TODO Indicate somehow that trySample should always succeed!
struct UnionDdf: public Ddf {
    vector< unique_ptr<Ddf>, segregated_allocator_class<unique_ptr<Ddf>> > components;
    vector< float, segregated_allocator_class<float> > weights;
    vector< unique_ptr<Ddf>, segregated_allocator_class<unique_ptr<Ddf>> > owning_pointers;
    UnionDdf(){
    }
    // weight eqals to sum of weights
    virtual glm::vec3 trySample() const override;
    virtual float value( vec3 arg ) const override;
};

template<class P>
SuperpositionDdf<P>::SuperpositionDdf(P _source, P _dest)
    :source(move(_source)), dest(move(_dest)){

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

template<class P>
vec3 SuperpositionDdf<P>::trySample() const {

    vec3 x = source->trySample();
    if(x == vec3())         // fail if fail
        return x;

    bool hit = p_hit( dest->value( x ) / dest->max_value );

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

using namespace ::detail;

std::unique_ptr<Ddf> unite(unique_ptr<Ddf> a, float ka, unique_ptr<Ddf> b, float kb){
    using namespace ::detail;

    if(a==nullptr)
        return unite(make_unique<UnionDdf>(), 0.0f, move(b), 1.0f);
    else if(b==nullptr)
        return unite(make_unique<UnionDdf>(), 0.0f, move(a), 1.0f);

    UnionDdf* ua = dynamic_cast<UnionDdf*>(a.get());
    UnionDdf* ub = dynamic_cast<UnionDdf*>(b.get());

    if(!ua && ub)
        return unite(move(b), kb, move(a), ka);

    else if(ua && ub) {

        if(ua->components.size()==0 && ka != 0.0f)
            return unite(nullptr, 0.0f, move(b), kb);

        // add 2 arrays
        std::unique_ptr<UnionDdf> res = std::make_unique<UnionDdf>();  // empty

        // copy ua components and weights
        std::move(ua->components.begin(), ua->components.end(), res->components.end());
        std::move(ua->owning_pointers.begin(), ua->owning_pointers.end(), res->owning_pointers.end());
        std::for_each(ua->weights.begin(), ua->weights.end(), [&res,ka,kb](float k){
            res->weights.push_back(k*ka/(ka+kb));
        });

        // copy b components and weights
        std::move(ub->components.begin(), ub->components.end(), res->components.end());
        std::move(ub->owning_pointers.begin(), ub->owning_pointers.end(), res->owning_pointers.end());
        std::for_each(ub->weights.begin(), ub->weights.end(), [&res,ka,kb](float k){
            res->weights.push_back(k*kb/(ka+kb));
        });

        return res;
    }
    else if(ua && !ub){

        if(ua->components.size()==0 && ka != 0.0f)
            return unite(nullptr, 0.0f, move(b), kb);

        // add b to array ua
        std::unique_ptr<UnionDdf> res(ua);
        a.release();
        // re-weigh
        std::for_each(res->weights.begin(), res->weights.end(), [ka,kb](float& k){
            k*=ka/(ka+kb);
        });
        res->components.push_back(move(b));
        res->weights.push_back(kb/(ka+kb));
        return res;

    }
    else {

        // add two simple distributions
        std::unique_ptr<UnionDdf> res = std::make_unique<UnionDdf>();
        res->components.push_back(move(a));
        res->weights.push_back(ka/(ka+kb));
        res->components.push_back(move(b));
        res->weights.push_back(kb/(ka+kb));
        return res;

    }// else
}

// NB res and args can be null!
unique_ptr<Ddf> chain(unique_ptr<Ddf> source, unique_ptr<Ddf> dest){

    using namespace ::detail;

    if(!source)
        return dest;
    if(!dest)
        return source;

    UnionDdf* u_source = dynamic_cast<UnionDdf*>(source.get());
    UnionDdf* u_dest = dynamic_cast<UnionDdf*>(dest.get());

    if( u_source ){
        if( u_dest ){
            // make cross-product
            // TODO do this algorithm nicer?!
            unique_ptr<UnionDdf> res = std::make_unique<UnionDdf>();
            for( unique_ptr<Ddf>& src: u_source->components ){
                size_t i;
                i = 0;
                for( unique_ptr<Ddf>& dst: u_dest->components){

                    size_t j;
                    j = 0;

                    unique_ptr<Ddf> sup = make_unique<SuperpositionDdf<Ddf*>>(src.get(), dst.get());
                    res->components.push_back(move(sup));
                    res->weights.push_back(u_source->weights[i]*u_dest->weights[j]);

                    ++j;
                }// for dst
                res->owning_pointers.push_back(move(src));
                ++i;
            }// for src

            move(u_dest->components.begin(), u_dest->components.end(), res->owning_pointers.end());

            return res;
        }
        else{
            // apply b to every a
            std::unique_ptr<UnionDdf> res = std::make_unique<UnionDdf>();
            // TODO normal for with i
            size_t i=0;
            for( unique_ptr<Ddf>& c: u_source->components ){
                unique_ptr<Ddf> sup = make_unique<SuperpositionDdf<Ddf*>>(c.get(), dest.get());
                res->components.push_back( move(sup) );
                res->weights.push_back(u_source->weights[i++]);
                res->owning_pointers.push_back(move(c));
            }// for
            return res;
        }
    }
    else if( u_dest ){
        // apply a to every b
        std::unique_ptr<UnionDdf> res = std::make_unique<UnionDdf>();
        // TODO normal for with i
        size_t i=0;
        for( unique_ptr<Ddf>& c: u_dest->components ){
            unique_ptr<Ddf> sup = make_unique<SuperpositionDdf<Ddf*>>(source.get(), c.get());
            res->components.push_back( move(sup) );
            res->weights.push_back(u_dest->weights[i++]);
            res->owning_pointers.push_back(move(c));
        }// for
        return res;
    }
    else{
        unique_ptr<Ddf> res = make_unique<SuperpositionDdf<unique_ptr<Ddf>>>(move(source), move(dest));
        return res;
    }
}

// TODO Add free functions with dynamic_cast for classes above!
