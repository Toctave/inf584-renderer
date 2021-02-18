#include "Shape.hpp"

Shape::Shape(const Primitive* primitive, const Material* material)
    : material_(material), primitive_(primitive) {
}

const Primitive* Shape::primitive() const {
    return primitive_;
}

const Material* Shape::material() const {
    return material_;
}

bool Shape::ray_intersect(const Ray& ray, Intersect& intersect) const {
    bool result = primitive_->ray_intersect(ray, intersect);

    if (result) {
        intersect.material = material_;
        intersect.shape = this;
    }

    return result;
}
