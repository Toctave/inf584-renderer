#pragma once

#include "Intersect.hpp"
#include "Vec.hpp"
#include "Color.hpp"

enum SurfaceType { EYE, DIFFUSE, SPECULAR, LIGHT, ANY, REPEAT };

class BRDF {
public:
    virtual RGBColor f(const Intersect& itx,
		       const Vec3& wi,
		       const Vec3& wo) const {
        return RGBColor();
    };
    virtual RGBColor emit(const Vec3& point,
                          const Vec3& wo) const {
        return RGBColor();
    }

    virtual SurfaceType surface_type() const = 0;
    virtual ~BRDF() {};
};

class LambertBRDF : public BRDF {
private:
    RGBColor albedo_;

public:
    LambertBRDF(const RGBColor& albedo);
    virtual RGBColor f(const Intersect& itx,
                          const Vec3& wi,
                          const Vec3& wo) const;
    virtual SurfaceType surface_type() const;
};

class MicrofacetBRDF : public BRDF {
private:
    float specular_ratio_;
    float roughness_;
    float alpha_;
    float alpha2_;
    float k_;
    float f0_;

public:
    MicrofacetBRDF(float specular_ratio, float roughness, float ior);
    virtual RGBColor f(const Intersect& itx,
                          const Vec3& wi,
                          const Vec3& wo) const;
    virtual SurfaceType surface_type() const;
};

class EmissionBRDF : public BRDF {
private:
    RGBColor irradiance_;

public:
    EmissionBRDF(const RGBColor& irradiance);
    virtual RGBColor emit(const Vec3& itx,
                          const Vec3& wo) const override;
    virtual SurfaceType surface_type() const;
};
