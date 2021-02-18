#include "Light.hpp"

PointLight::PointLight(const Vec3& position, const RGBColor& color, float intensity)
    : position_(position), intensity_(color * intensity) {
}

LightSample PointLight::sample(const Vec3& point) const {
    Vec3 to_light = position_ - point;

    return LightSample(
        Ray::segment(point, position_),
        intensity_ / to_light.norm_squared()
    );
}
