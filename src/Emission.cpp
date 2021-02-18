#include "Material.hpp"

Emission::Emission(const RGBColor& irradiance)
    : irradiance_(irradiance) {
}

RGBColor Emission::emit(const Intersect& itx,
                      const Vec3& wo) const {
    return irradiance_ / static_cast<float>(2.0f * M_PI);
}