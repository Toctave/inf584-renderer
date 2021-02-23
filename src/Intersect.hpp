#pragma once

#include "Ray.hpp"
#include <limits>
#include <cassert>

class Shape;
class Material;

struct Intersect {
    float t;

    Vec3 point;
    Vec3 wo;
    
    const Shape* shape;
    const Material* material;
    
    Vec3 normal;
    // Vec2 uv;

    Intersect()
        : t(INFTY),
          shape(nullptr),
          material(nullptr) {
    }
};
