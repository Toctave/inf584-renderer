#pragma once

#include "Vec.hpp"
#include "constants.hpp"

struct Ray {
    Vec3 o;
    Vec3 d;
    mutable float tmax;

    static Ray segment(Vec3 v1, Vec3 v2) {
        Ray result(v1, v2 - v1);
        result.tmax = 1.0f;
        
        return result;
    }

    Ray(const Vec3& o, const Vec3& d)
        : o(o), d(d), tmax(INFTY) {
    }

    Vec3 at(float t) const {
        return o + t * d;
    }

    Vec3 target() const {
	return at(tmax);
    }
};
