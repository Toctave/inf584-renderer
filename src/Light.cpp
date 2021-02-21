#include "Light.hpp"
#include "Material.hpp"

PointLight::PointLight(const Vec3& position, const RGBColor& color, float intensity)
    : position_(position), intensity_(color * intensity) {
}

LightSample PointLight::sample(const Vec3& point) const {
    Vec3 to_light = position_ - point;

    return LightSample(
        Ray::segment(point, position_),
        intensity_ / to_light.norm_squared(),
        1.0f
    );
}


AreaLight::AreaLight(const Shape* shape)
    : shape_(shape) {
}

LightSample AreaLight::sample(const Vec3& point) const {
    float pdf;
    Vec3 on_light = shape_->primitive()->sample(pdf);
    
    Ray itx_ray(point, on_light - point);
    Intersect itx;
    if (!shape_->ray_intersect(itx_ray, itx)) {
        // sampled a point on the silhouette of the shape,
        // ignore it
        return LightSample(
            Ray::segment(point, on_light),
            RGBColor(),
            1.0f
        );
    }

    on_light = itx_ray.at(itx.t);
    if (itx.t < EPSILON) {
        throw std::runtime_error("Shadow ray error");
    }

    Vec3 wi = (on_light - point).normalized();
    float solid_angle_pdf = pdf * (on_light - point).norm_squared()
        / fabs(dot(itx.normal, -wi));

    return LightSample(
        Ray::segment(point, on_light),
        shape_->material()->emit(on_light, -wi),
        solid_angle_pdf
    );
}
