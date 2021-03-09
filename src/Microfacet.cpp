#include "Material.hpp"

#include <cmath>

MicrofacetMaterial::MicrofacetMaterial(const RGBColor& albedo, float specular_ratio, float roughness, float ior)
    : albedo_(albedo),
      specular_ratio_(specular_ratio),
      roughness_(roughness),
      alpha_(roughness * roughness),
      alpha2_(alpha_ * alpha_) {
    k_ = roughness_ + 1.0f;
    k_ = k_ * k_ / 8.0f;

    f0_ = (ior - 1.0f) / (ior + 1.0f);
    f0_ *= f0_;
}

// https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf
RGBColor MicrofacetMaterial::brdf(const Intersect& itx,
                                  const Vec3& wi,
                                  const Vec3& wo) const {
    Vec3 h = (wi + wo).normalized();

    float ndoth = dot(itx.normal, h);
    float ndoth2 = ndoth * ndoth;

    float tmp1 = (ndoth2 * (alpha2_ - 1.0f) + 1.0f);

    float d = alpha2_ / (M_PI * tmp1 * tmp1);

    float n_dot_wo = dot(itx.normal, wo);
    float n_dot_wi = dot(itx.normal, wo);

    float g1wo = n_dot_wo / (n_dot_wo * (1.0f - k_) + k_);
    float g1wi = n_dot_wi / (n_dot_wi * (1.0f - k_) + k_);

    float g = g1wo * g1wi;


    float wo_dot_h = dot(wo, h);

    float fresnel_exponent = (-5.55473f * wo_dot_h - 6.98316f) * wo_dot_h;
    float fresnel = f0_
        + ((1.0f - f0_)
           * std::pow(2.0f, fresnel_exponent));

    float brdf_val = d * fresnel * g / (4.0f * n_dot_wi * n_dot_wo);

    // return RGBColor(brdf_val);
    return albedo_ / static_cast<float>(M_PI) + RGBColor(brdf_val);
}

SurfaceType MicrofacetMaterial::surface_type() const {
    return SurfaceType::SPECULAR;
}
