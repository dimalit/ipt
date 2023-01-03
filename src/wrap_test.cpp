#include <memory>

#include<iostream>
class vec3 {};

class SphericalDistribution {
public:
    virtual vec3 randRay() const = 0;
};

namespace detail{
    class SphericalDistributionOperations;
}

class spherical_distribution{
private:
    friend class detail::SphericalDistributionOperations;
    typedef SphericalDistribution interface;
    std::shared_ptr<interface> pimpl;

    spherical_distribution( std::shared_ptr<SphericalDistribution> src )
        :pimpl( src ){}

public:
    vec3 rand_ray() const {
        return pimpl->randRay();
    }
};

namespace detail{

    class SphericalDistributionOperations{
    public:
        static spherical_distribution wrap(SphericalDistribution* src){
            return spherical_distribution( std::shared_ptr<SphericalDistribution>( src ) );
        }
        static spherical_distribution wrap(std::unique_ptr<SphericalDistribution> src){
            return spherical_distribution( std::move( src ) );
        }
        static spherical_distribution wrap(std::shared_ptr<SphericalDistribution> src){
            return spherical_distribution( src );
        }

        static SphericalDistribution* unwrap( spherical_distribution src ){
            return src.pimpl.get();
        }
    };
};

class Distribution1: public SphericalDistribution {
    vec3 randRay() const override {
        std::cout << "d1" << std::endl;
        return vec3();
    };
};

class Distribution2: public SphericalDistribution {
    vec3 randRay() const override {
        std::cout << "d2" << std::endl;
        return vec3();
    };
};

vec3 x_rand_ray(spherical_distribution a, spherical_distribution b) {
    SphericalDistribution* aa = detail::SphericalDistributionOperations::unwrap(a);
    SphericalDistribution* bb = detail::SphericalDistributionOperations::unwrap(b);
    Distribution1* d1a = dynamic_cast<Distribution1*>(aa);
    Distribution1* d1b = dynamic_cast<Distribution1*>(bb);
    Distribution2* d2a = dynamic_cast<Distribution2*>(aa);
    Distribution2* d2b = dynamic_cast<Distribution2*>(bb);

    std::cout << "Combining " << (d1a ? "D1" : d2a ? "D2" : "HZ") << " with " << (d1b ? "D1" : d2b ? "D2" : "HZ") << std::endl;
    return vec3();
}

typedef std::shared_ptr<SphericalDistribution> sspherical_distribution;
vec3 x_rand_ray(sspherical_distribution a, sspherical_distribution b) {
    SphericalDistribution* aa = a.get();
    SphericalDistribution* bb = b.get();
    Distribution1* d1a = dynamic_cast<Distribution1*>(aa);
    Distribution1* d1b = dynamic_cast<Distribution1*>(bb);
    Distribution2* d2a = dynamic_cast<Distribution2*>(aa);
    Distribution2* d2b = dynamic_cast<Distribution2*>(bb);

    std::cout << "Combining " << (d1a ? "D1" : d2a ? "D2" : "HZ") << " with " << (d1b ? "D1" : d2b ? "D2" : "HZ") << std::endl;
    return vec3();
}

using namespace std;
using namespace detail;

void test(){
    SphericalDistribution* d1 = new Distribution1();
    SphericalDistribution* d2 = new Distribution2();

    spherical_distribution dd1 = SphericalDistributionOperations::wrap( d1 );
    spherical_distribution dd2 = SphericalDistributionOperations::wrap( d2 );

    (void) dd1.rand_ray();
    (void) dd2.rand_ray();

    (void) x_rand_ray(dd2, dd2);
}

void test2(){
    SphericalDistribution* d1 = new Distribution1();
    SphericalDistribution* d2 = new Distribution2();

    sspherical_distribution dd1(d1);
    sspherical_distribution dd2(d2);

    (void) dd1->randRay();
    (void) dd2->randRay();

    (void) x_rand_ray(dd2, dd1);
}
