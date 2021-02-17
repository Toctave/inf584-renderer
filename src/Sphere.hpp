#pragma once

#include "Vec.hpp"
#include "Shape.hpp"

class Sphere {
private:
    Vec3 center_;
    float radius_;

public:
    Sphere(Vec3 center, float radius);

    virtual bool ray_intersect(const Ray& ray);
    virtual bool ray_intersect(const Ray& ray, Intersect& intersect);
};
