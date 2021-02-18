#include "Sphere.hpp"

Sphere::Sphere(Vec3 center, float radius)
    : center_(center), radius_(radius) {
}

float Sphere::ray_intersect(const Ray& ray) const {
    float a = ray.d.norm_squared();
    float b = 2.0f * dot(ray.o - center_, ray.d);
    float c = (ray.o - center_).norm_squared() - radius_ * radius_;

    float disc = b * b - 4 * a * c;
    
    if (disc < 0.0f) {
        return INFTY; // no intersection
    }

    float sqdisc = std::sqrt(disc);

    float t0 = (-b - sqdisc) / (2.0f * a);

    if (t0 >= 0.0f && t0 < ray.tmax) {
        return t0;
    }

    float t1 = (-b + sqdisc) / (2.0f * a);

    if (t1 >= 0.0f && t1 < ray.tmax) {
        return t1;
    }

    return INFTY;
}

bool Sphere::ray_intersect(const Ray& ray, Intersect& intersect) const {
    float a = ray.d.norm_squared();
    float b = 2.0f * dot(ray.o - center_, ray.d);
    float c = (ray.o - center_).norm_squared() - radius_ * radius_;

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
    } else {
        float t1 = (-b + sqdisc) / (2.0f * a);

        if (t1 >= 0.0f && t1 < ray.tmax) {
            hit = true;
            intersect.t = t1;
        }
    }

    if (hit) {
        Vec3 point = ray.at(intersect.t);
        intersect.normal = (point - center_) / radius_;
    }

    return hit;
}
