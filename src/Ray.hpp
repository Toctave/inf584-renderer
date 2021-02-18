#pragma once

#include "Vec.hpp"
#include "constants.hpp"

struct Ray {
    Vec3 o;
    Vec3 d;
    float tmax;

    Ray(const Vec3& o, const Vec3& d)
        : d(d), o(o), tmax(INFTY) {
    }

    Vec3 at(float t) const {
        return o + t * d;
    }
};
