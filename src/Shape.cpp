#include "Shape.hpp"

#include "transform.hpp"

void Primitive::print() const {
    std::cout << "Primitive\n";
}

Shape::Shape(const Primitive* primitive, const Material* material)
    : material_(material), primitive_(primitive), transform_(1.0f), inv_transform_(1.0f) {
}

const Primitive* Shape::primitive() const {
    return primitive_;
}

const Material* Shape::material() const {
    return material_;
}


bool Shape::ray_intersect(const Ray& ray, Intersect& intersect) const {
    Ray transformed_ray(transform_point(inv_transform_, ray.o), transform_vector(inv_transform_, ray.d));
    transformed_ray.tmax = ray.tmax;
    
    bool result = primitive_->ray_intersect(transformed_ray, intersect);
    
    if (result) {
	intersect.normal = transform_vector(transform_, intersect.normal);
        intersect.point = transform_point(transform_, transformed_ray.at(intersect.t));
        intersect.wo = transform_vector(transform_, -transformed_ray.d);
        intersect.shape = this;
        intersect.material = material_;
    }

    return result;
}

void Shape::set_transform(const Matrix4& transform) {
    transform_ = transform;
    inv_transform_ = transform.inverse();
}
