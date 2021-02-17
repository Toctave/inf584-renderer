#pragma once

#include "Ray.hpp"

class Shape;

struct Intersect {
    const Ray& incoming;
    
    float t;

    Shape* shape;
    Vec3 normal;
    // Vec2 uv;

    Intersect(const Ray& r) : incoming(r), shape(nullptr) {}
};
