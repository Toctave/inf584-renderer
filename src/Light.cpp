#include "Light.hpp"
#include "Material.hpp"

Light::~Light() {
}

PointLight::PointLight(const Vec3& position, const RGBColor& color, float intensity)
    : position_(position), intensity_(color * intensity) {
}

LightSample PointLight::sample(const Vec3& point) const {
    Vec3 to_light = position_ - point;

    return LightSample(
        Ray::segment(point, position_),
        intensity_ / norm_squared(to_light),
        1.0f
    );
}

bool PointLight::is_shape(const Shape* shape) const {
    return false;
}

AreaLight::AreaLight(const Shape* shape)
    : shape_(shape) {
}

bool AreaLight::is_shape(const Shape* shape) const {
    return shape == shape_;
}

LightSample AreaLight::sample(const Vec3& point) const {
    float pdf;
    Vec3 on_light = shape_->sample(pdf);
    
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

    on_light = itx_ray.target();

    Vec3 wi = (on_light - point).normalized();
    float solid_angle_pdf = pdf * norm_squared(on_light - point)
        / fabs(dot(itx.normal, wi));

    Ray shadow_ray = Ray::segment(point, on_light);

    RGBColor emitted;
    for (const BRDF* brdf : shape_->material()->brdfs()) {
	emitted += brdf->emit(on_light, -wi);
    }
    
    return LightSample(
        shadow_ray,
        emitted, 
        solid_angle_pdf
    );
}
