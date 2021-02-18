#include "Light.hpp"

PointLight::PointLight(const Vec3& position, const RGBColor& color, float intensity)
    : position_(position), intensity_(color * intensity) {
}

LightSample PointLight::sample(const Vec3& point) const {
    LightSample sample;

    Vec3 to_light = position_ - point;

    sample.wi = to_light.normalized();
    sample.intensity = intensity_ / to_light.norm_squared();

    return sample;
}
