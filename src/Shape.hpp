#pragma once

#include "Intersect.hpp"

class Shape {
public:
    virtual bool ray_intersect(const Ray& ray) const = 0;
    virtual bool ray_intersect(const Ray& ray, Intersect& intersect) const = 0;
};
