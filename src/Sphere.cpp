#include "Sphere.hpp"
#include "Sampling.hpp"

#include <cassert>

void Sphere::print() const {
    std::cout << "Sphere(" << center_ << ", " << radius_ << ")\n";
}

Sphere::Sphere(Vec3 center, float radius)
    : center_(center), radius_(radius) {
}

bool Sphere::ray_intersect(const Ray& ray) const {
    float a = norm_squared(ray.d);
    float b = 2.0f * dot(ray.o - center_, ray.d);
    float c = norm_squared(ray.o - center_) - radius_ * radius_;

    float disc = b * b - 4 * a * c;
    
    if (disc < 0.0f) {
        return false; // no intersection
    }

    float sqdisc = std::sqrt(disc);

    float t0 = (-b - sqdisc) / (2.0f * a);

    if (t0 >= 0.0f && t0 < ray.tmax) {
	ray.tmax = t0;
        return true;
    }

    float t1 = (-b + sqdisc) / (2.0f * a);

    if (t1 >= 0.0f && t1 < ray.tmax) {
	ray.tmax = t1;
        return true;
    }

    return false;
}

float Sphere::area() const {
    return 4.0f * M_PI * radius_ * radius_;
}

bool Sphere::ray_intersect(const Ray& ray, Intersect& intersect) const {
    float a = norm_squared(ray.d);
    float b = 2.0f * dot(ray.o - center_, ray.d);
    float c = norm_squared(ray.o - center_) - radius_ * radius_;

    float disc = b * b - 4 * a * c;
    
    if (disc < 0.0f) {
        return false; // no intersection
    }

    bool hit = false;
    float sqdisc = std::sqrt(disc);
    
    float t0 = (-b - sqdisc) / (2.0f * a);

    if (t0 >= 0.0f && t0 < ray.tmax) {
        hit = true;
        intersect.t = t0;
	ray.tmax = t0;
    } else {
        float t1 = (-b + sqdisc) / (2.0f * a);

        if (t1 >= 0.0f && t1 < ray.tmax) {
            hit = true;
            intersect.t = t1;
	    ray.tmax = t1;
        }
    }

    if (hit) {
        Vec3 point = ray.at(intersect.t);
        intersect.normal = (point - center_) / radius_;
    }

    return hit;
}

Vec3 Sphere::sample(float& pdf) const {
    pdf = 1.0f / (4.0f * M_PI * radius_ * radius_);

    Vec3 p = center_ + sample_unit_sphere() * radius_;

    return p;
}
