#include "BRDF.hpp"
#include "Material.hpp"
#include "Sampling.hpp"

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

Vec3 LambertBRDF::sample_wi(const Intersect& itx, const Vec3& wo, float* pdf) const {
    Vec3 wi_sample = sample_hemisphere_cosine_weighted(pdf);

    return wi_sample[0] * itx.local_x
	+ wi_sample[1] * itx.local_y
	+ wi_sample[2] * itx.normal;
}

LambertMaterial::LambertMaterial(const RGBColor& albedo) {
    brdfs_.push_back(new LambertBRDF(albedo));
}
