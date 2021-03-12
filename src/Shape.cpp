#include "Shape.hpp"

#include "Transform.hpp"

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
    Ray transformed_ray = transform_ray(transform_.inverse(), ray);
    
    bool result = primitive_->ray_intersect(transformed_ray, intersect);
    
    if (result) {
	intersect.normal = transform_normal(transform_, intersect.normal);
        intersect.point = transform_point(transform_, transformed_ray.at(intersect.t));
        intersect.wo = transform_vector(transform_, -transformed_ray.d);
        intersect.shape = this;
        intersect.material = material_;

	intersect.normal.normalize();
	intersect.wo.normalize();
    }

    return result;
}

bool Shape::ray_intersect(const Ray& ray) const {
    Ray transformed_ray = transform_ray(transform_.inverse(), ray);
    
    return primitive_->ray_intersect(transformed_ray);
}

void Shape::set_transform(const Transform& transform) {
    transform_ = transform;
}

void Shape::set_transform(Transform&& transform) {
    transform_ = transform;
}

Vec3 Shape::sample(float& pdf) const {
    Vec3 result = primitive_->sample(pdf);

    return transform_point(transform_, result);
}

Primitive::~Primitive() {
}

