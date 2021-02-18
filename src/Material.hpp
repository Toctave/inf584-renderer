#pragma once

#include "Intersect.hpp"
#include "Vec.hpp"
#include "Color.hpp"

class Material {
public:
    virtual RGBColor brdf(const Intersect& itx,
                          const Vec3& wi,
                          const Vec3& wo) const {
        return RGBColor();
    };
    virtual RGBColor emit(const Intersect& itx,
                          const Vec3& wo) const {
        return RGBColor();
    }
};

class LambertMaterial : public Material {
private:
    RGBColor albedo_;

public:
    LambertMaterial(const RGBColor& albedo);
    virtual RGBColor brdf(const Intersect& itx,
                          const Vec3& wi,
                          const Vec3& wo) const;
};

class Emission : public Material {
private:
    RGBColor irradiance_;

public:
    Emission(const RGBColor& irradiance);
    virtual RGBColor emit(const Intersect& itx,
                          const Vec3& wo) const override;
};
