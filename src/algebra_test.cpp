#include <memory>
#include <vector>
#include <functional>

// GLOBAL TODO Use continuous BSDFs + small or even point lights.
// Approximate light contribution on surface as well as on slope!

struct vec3 {
    vec3 operator*(float) const {return vec3();}
};

struct mat3{
    vec3 operator*(vec3 arg) const {return arg;}
};

bool hit(float prob);

struct Narrow;

struct Distribution {
    // may return 0 if sampling failed
    // this is used to balance parts of union distribution
    virtual vec3 sample() const = 0;
    virtual float pdf( vec3 arg ) const = 0;
    virtual float integral(const Narrow&) const = 0;
    float weight;
};

struct Narrow: public Distribution {
    vec3 direction; // center
    // pdf = 1.0f / area;
};

struct Superposition: public Narrow {

    std::shared_ptr<const Distribution> a;
    std::shared_ptr<const Narrow> b;

    Superposition(std::shared_ptr<const Distribution> _a, std::shared_ptr<const Narrow> _b)
        :a(_a), b(_b){

        this->direction = _b->direction;
        this->weight = a->weight * a->integral(*b.get());
    }

    virtual vec3 sample() const {
        vec3 x = b->sample();
        return x * a->pdf(x);
    }
};

struct Sum: public Distribution {
    std::vector< std::shared_ptr<Distribution> > components;
    std::vector< float > weights;
    virtual vec3 sample() const = 0;
};

std::shared_ptr<Distribution> add(std::shared_ptr<Distribution> a, std::shared_ptr<Distribution> b, float ka){
    if(!dynamic_cast<Sum*>(a.get()) && dynamic_cast<Sum*>(b.get()))
        return add(b, a, 1.0f-ka);
    if(dynamic_cast<Sum*>(a.get())){
        if(dynamic_cast<Sum*>(b.get())){
            // add 2 arrays
        }
        else{
            // add b to array a
        }
    }
    else{
        // add 2 plain distributions
    }
}

std::shared_ptr<Distribution> apply(std::shared_ptr<Distribution> a, std::shared_ptr<Distribution> b){
    if(!dynamic_cast<Sum*>(a.get()) && dynamic_cast<Sum*>(b.get()))
        return apply(b, a);
    if(dynamic_cast<Sum*>(a.get())){
        if(dynamic_cast<Sum*>(b.get())){
            // make cross-product
            // NOTE TODO How to compute cross-measures!?
        }
        else{
            // apply b to every a
        }
    }
    else{
        // apply plain to plain
    }
}

struct Clip: public Continuous {
    std::shared_ptr<Continuous> origin;
    std::function<bool(vec3)> predicate;
    virtual vec3 sample() const {
        for(;;){
            vec3 x = origin->sample();
            if(predicate(x))
                return x;
        }
    }
    // TODO it will be not 1-normalized!!
    virtual float pdf(vec3 x) const {
        return origin->pdf(x);
    }
};

struct Transform: public Distribution {
    std::shared_ptr<Distribution> origin;
    mat3 transormation;
    virtual vec3 sample() const {
        vec3 x = origin->sample();
        return transormation * x;
    }
    // TODO Need separate implementations for Continuous and Singular
};

struct Rotate: public Distribution {
    std::shared_ptr<Transform> origin;
    Rotate(vec3 /*to*/){
        // set original transformation correctly
    }
};

// TODO Add free functions with dynamic_cast for classes above!
