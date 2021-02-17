#pragma once

#include "Vec.hpp"

struct Ray {
    Vec3 o;
    Vec3 d;

    Ray(const Vec3& o, const Vec3& d) : d(d), o(o) {
    }

    Vec3 at(float t) const {
        return o + t * d;
    }
};
