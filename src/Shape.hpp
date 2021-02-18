#pragma once

#include "Intersect.hpp"

class Primitive {
public:
    virtual float ray_intersect(const Ray& ray) const = 0;
    virtual bool ray_intersect(const Ray& ray, Intersect& intersect) const = 0;
    virtual Vec3 sample(float& pdf) const = 0;
};

class Shape {
private:
    const Material* material_;
    const Primitive* primitive_;

public:
    Shape(const Primitive* primitive, const Material* material);

    const Primitive* primitive() const;
    const Material* material() const;
    virtual bool ray_intersect(const Ray& ray, Intersect& intersect) const;
};
