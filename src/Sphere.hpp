#pragma once

#include "Vec.hpp"
#include "Shape.hpp"

class Sphere : public Primitive {
private:
    Vec3 center_;
    float radius_;

public:
    Sphere(Vec3 center, float radius);

    virtual bool ray_intersect(const Ray& ray) const override;
    virtual bool ray_intersect(const Ray& ray, Intersect& intersect) const override;
    virtual Vec3 sample(float& pdf) const override;
    virtual void print() const override;
};
