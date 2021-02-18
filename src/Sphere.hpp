#pragma once

#include "Vec.hpp"
#include "Shape.hpp"

class Sphere : public Primitive {
private:
    Vec3 center_;
    float radius_;

public:
    Sphere(Vec3 center, float radius);

    virtual float ray_intersect(const Ray& ray) const ;
    virtual bool ray_intersect(const Ray& ray, Intersect& intersect) const;
};
