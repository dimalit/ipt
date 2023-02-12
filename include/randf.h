#ifndef RANDF_H
#define RANDF_H

#include <cstdlib>

static float randf() {
    float res = drand48();
    while(res==1.0f)
        res = drand48();
    return res;
}

#endif // RANDF_H
