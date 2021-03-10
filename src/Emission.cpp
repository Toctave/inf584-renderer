#include "BRDF.hpp"
#include "Material.hpp"
#include "Sampling.hpp"

EmissionBRDF::EmissionBRDF(const RGBColor& irradiance)
    : irradiance_(irradiance) {
}

RGBColor EmissionBRDF::emit(const Vec3& point,
                        const Vec3& wo) const {
    return irradiance_ / static_cast<float>(2.0f * M_PI);
}

SurfaceType EmissionBRDF::surface_type() const {
    return SurfaceType::LIGHT;
}

Vec3 EmissionBRDF::sample_wi(const Intersect& itx, const Vec3& wo, float* pdf) const {
    Vec3 wi_sample = sample_hemisphere_cosine_weighted(pdf);

    return wi_sample[0] * itx.local_x
	+ wi_sample[1] * itx.local_y
	+ wi_sample[2] * itx.normal;
}

Emission::Emission(const RGBColor& irradiance) {
    brdfs_.push_back(new EmissionBRDF(irradiance));
}
