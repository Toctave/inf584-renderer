#pragma once

#include "Intersect.hpp"
#include "Matrix.hpp"

class Primitive {
public:
    virtual bool ray_intersect(const Ray& ray) const = 0;
    virtual bool ray_intersect(const Ray& ray, Intersect& intersect) const = 0;
    virtual Vec3 sample(float& pdf) const = 0;
    virtual void print() const;
};

class Shape {
private:
    const Material* material_;
    const Primitive* primitive_;
    Matrix4 transform_;
    Matrix4 inv_transform_;

public:
    Shape(const Primitive* primitive, const Material* material);

    const Primitive* primitive() const;
    const Material* material() const;
    virtual bool ray_intersect(const Ray& ray, Intersect& intersect) const;

    void set_transform(const Matrix4& transform);
};

