#pragma once

#include "Ray.hpp"
#include <limits>
#include <cassert>

class Shape;
class Material;

struct Intersect {
    float t;

    const Ray* ray;
    const Shape* shape;
    const Material* material;
    
    Vec3 normal;
    // Vec2 uv;

    Intersect()
        : t(INFTY),
          ray(nullptr),
          shape(nullptr),
          material(nullptr) {
    }
};
