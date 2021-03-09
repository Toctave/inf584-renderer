#include "BRDF.hpp"
#include "Material.hpp"

LambertBRDF::LambertBRDF(const RGBColor& albedo)
    : albedo_(albedo) {
}

RGBColor LambertBRDF::f(const Intersect& itx,
			const Vec3& wi,
			const Vec3& wo) const {
    return albedo_ / static_cast<float>(M_PI);
}

SurfaceType LambertBRDF::surface_type() const {
    return SurfaceType::DIFFUSE;
}

LambertMaterial::LambertMaterial(const RGBColor& albedo) {
    brdfs_.push_back(new LambertBRDF(albedo));
}
