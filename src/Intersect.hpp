#pragma once

#include "Ray.hpp"
#include <limits>

class Shape;
class Material;

struct Intersect {
    const Ray& incoming;
    
    float t;

    const Shape* shape;
    const Material* material;
    
    Vec3 normal;
    // Vec2 uv;

    Intersect(const Ray& r)
        : incoming(r),
          shape(nullptr),
          material(nullptr) {
    }

};
