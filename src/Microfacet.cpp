#include "BRDF.hpp"
#include "Material.hpp"
#include "Sampling.hpp"

#include <cmath>

MicrofacetBRDF::MicrofacetBRDF(float roughness, float ior)
    : roughness_(roughness),
      alpha_(roughness * roughness),
      alpha2_(alpha_ * alpha_) {
    k_ = roughness_ + 1.0f;
    k_ = k_ * k_ / 8.0f;

    f0_ = (ior - 1.0f) / (ior + 1.0f);
    f0_ *= f0_;
}

// https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf
RGBColor MicrofacetBRDF::f(const Intersect& itx,
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

    if (g == 0.0f) {
	return RGBColor();
    }

    float wo_dot_h = dot(wo, h);

    float fresnel_exponent = (-5.55473f * wo_dot_h - 6.98316f) * wo_dot_h;
    float fresnel = f0_
        + ((1.0f - f0_)
           * std::pow(2.0f, fresnel_exponent));

    float brdf_val = d * fresnel * g / (4.0f * n_dot_wi * n_dot_wo);

    return RGBColor(brdf_val);
}

SurfaceType MicrofacetBRDF::surface_type() const {
    return SurfaceType::SPECULAR;
}

Vec3 reflect(const Vec3& v, const Vec3& ref) {
    return 2.0f * dot(v, ref) * ref - v;
}

// https://schuttejoe.github.io/post/ggximportancesamplingpart1/
// https://agraphicsguy.wordpress.com/2015/11/01/sampling-microfacet-brdf/
Vec3 MicrofacetBRDF::local_wi_sample(const Vec3& wo, float* pdf) const {
    Vec2 rnd = sample_unit_square();

    float theta = std::acos(std::sqrt((1 - rnd[0]) / ((alpha2_ - 1.0f) * rnd[0] + 1.0f)));
    float phi = 2 * M_PI * rnd[1];

    float st = std::sin(theta);

    Vec3 wm = Vec3(st * std::cos(phi),
		   st * std::sin(phi),
		   std::cos(theta));
    
    Vec3 wi = reflect(wo, wm);

    float wm_dot_n = wm[2];
    float exp = (alpha2_ - 1.0f) * wm_dot_n * wm_dot_n + 1.0f;
    float d = alpha2_ / (M_PI * exp * exp);
    *pdf = d * wm_dot_n / (4.0f * wo[2]);

    return wi;
}

Vec3 MicrofacetBRDF::sample_wi(const Intersect& itx, const Vec3& wo, float* pdf) const {
    Vec3 local_wo(
	dot(wo, itx.local_x),
	dot(wo, itx.local_y),
	dot(wo, itx.normal)
	);
    
    Vec3 wi_sample = local_wi_sample(local_wo, pdf);

    return wi_sample[0] * itx.local_x
	+ wi_sample[1] * itx.local_y
	+ wi_sample[2] * itx.normal;
}

MicrofacetMaterial::MicrofacetMaterial(const RGBColor& albedo, float roughness, float ior) {
    brdfs_.push_back(new LambertBRDF(albedo));
    brdfs_.push_back(new MicrofacetBRDF(roughness, ior));
}
