#pragma once

#include "Ray.hpp"
#include <limits>
#include <cassert>

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
          t(INFTY),
          shape(nullptr),
          material(nullptr) {
    }

    Intersect& operator=(const Intersect& other) {
        assert(&incoming == &other.incoming);

        t = other.t;
        shape = other.shape;
        material = other.material;
        normal = other.normal;

        return *this;
    }

    Intersect(const Intersect& other) :
        incoming(other.incoming),
        t(other.t),
        shape(other.shape),
        material(other.material) {
    }
};
