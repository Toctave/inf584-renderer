#include "BRDF.hpp"
#include "Material.hpp"

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

Emission::Emission(const RGBColor& irradiance) {
    brdfs_.push_back(new EmissionBRDF(irradiance));
}
