#pragma once

#include "Ray.hpp"
#include <limits>

class Shape;

struct Intersect {
    const Ray& incoming;
    
    float t;

    const Shape* shape;
    Vec3 normal;
    // Vec2 uv;

    Intersect(const Ray& r)
        : incoming(r),
          shape(nullptr) {
    }

};
