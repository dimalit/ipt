#include "GeometrySmallPt.h"

#include <libddf/ddf_detail.h>

#include <glm/geometric.hpp>

#include <cmath>

using namespace glm;
using namespace std;

struct Sphere {
  double rad;       // radius
  vec3 p, e, c;      // position, emission
  Sphere(double rad_, const vec3& p_):
    rad(rad_), p(p_) {}
  double intersect(const vec3& ro, const vec3& rd) const { // returns distance, 0 if nohit
    vec3 op = p-ro; // Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0
    double t, eps=1e-4, b=dot(op, rd), det=b*b-dot(op,op)+rad*rad;
    if (det<0) return 0; else det=sqrt(det);
    return (t=b-det)>eps ? t : ((t=b+det)>eps ? t : 0);
  }
};

Sphere spheres[] = {//Scene: radius, position, emission, color, material
  Sphere(1e3, vec3( 1e3+1,40.8,81.6)),//Left
  Sphere(1e3, vec3(-1e3+99,40.8,81.6)),//Rght
  Sphere(1e3, vec3(50,40.8, 1e3)     ),//Back
//  Sphere(1e5, vec3(50,40.8,-1e5+170) ),//Frnt
  Sphere(1e3, vec3(50, 1e3, 81.6)    ),//Botm
  Sphere(1e3, vec3(50,-1e3+81.6,81.6)),//Top
  Sphere(16.5,vec3(27,16.5,47)       ),
  Sphere(16.5,vec3(73,16.5,78)       )//Glas
};

optional<surface_intersection> GeometrySmallPt::traceRay(vec3 origin, vec3 direction) const {
    double min_t = std::numeric_limits<double>::infinity();
    const Sphere* min_s;
    for(const Sphere& s:spheres){
        double t = s.intersect(origin, direction);
        if(t!=0.0){
            if(t<min_t){
                min_t = t;
                min_s = &s;
            }// if min
        }// if not 0
    }// for

    if(isfinite(min_t)){
        surface_intersection inter;
        inter.curvature=-1.0/min_s->rad;
        inter.position = origin+direction*(float)min_t;
        inter.normal = min_s->rad < 100 ? normalize(inter.position-min_s->p) : -normalize(inter.position-min_s->p);
        unique_ptr<Ddf> dis = make_unique<CosineDdf>();
        unique_ptr<RotateDdf> rotate = make_unique<RotateDdf>(move(dis), inter.normal);
        inter.sdf = move(rotate);
        return move(inter);
    }// if finite

    return {};
}
