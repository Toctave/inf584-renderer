#include "Shape.hpp"

void Primitive::print() const {
    std::cout << "Primitive\n";
}

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
        intersect.ray = &ray;
        intersect.shape = this;
        intersect.material = material_;
    }

    return result;
}
