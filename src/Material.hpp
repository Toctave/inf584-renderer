#pragma once

#include "Intersect.hpp"
#include "Vec.hpp"
#include "Color.hpp"

enum SurfaceType { EYE, DIFFUSE, SPECULAR, LIGHT, ANY, REPEAT };

class Material {
public:
    virtual RGBColor brdf(const Intersect& itx,
                          const Vec3& wi,
                          const Vec3& wo) const {
        return RGBColor();
    };
    virtual RGBColor emit(const Vec3& point,
                          const Vec3& wo) const {
        return RGBColor();
    }

    virtual SurfaceType surface_type() const = 0;
};

class LambertMaterial : public Material {
private:
    RGBColor albedo_;

public:
    LambertMaterial(const RGBColor& albedo);
    virtual RGBColor brdf(const Intersect& itx,
                          const Vec3& wi,
                          const Vec3& wo) const;
    virtual SurfaceType surface_type() const;
};

class MicrofacetMaterial : public Material {
private:
    RGBColor albedo_;
    float specular_ratio_;
    float roughness_;
    float alpha_;
    float alpha2_;
    float k_;
    float f0_;

public:
    MicrofacetMaterial(const RGBColor& albedo, float specular_ratio, float roughness, float f0);
    virtual RGBColor brdf(const Intersect& itx,
                          const Vec3& wi,
                          const Vec3& wo) const;
    virtual SurfaceType surface_type() const;
};

class Emission : public Material {
private:
    RGBColor irradiance_;

public:
    Emission(const RGBColor& irradiance);
    virtual RGBColor emit(const Vec3& itx,
                          const Vec3& wo) const override;
    virtual SurfaceType surface_type() const;
};
