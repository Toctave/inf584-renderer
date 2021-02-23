#include "Material.hpp"

LambertMaterial::LambertMaterial(const RGBColor& albedo)
    : albedo_(albedo) {
}

RGBColor LambertMaterial::brdf(const Intersect& itx,
                               const Vec3& wi,
                               const Vec3& wo) const {
    return albedo_ / static_cast<float>(M_PI);
}

SurfaceType LambertMaterial::surface_type() const {
    return SurfaceType::DIFFUSE;
}
