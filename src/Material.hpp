#pragma once
#include "BRDF.hpp"

#include <vector>

class Material {
protected:
    std::vector<const BRDF*> brdfs_;
public:
    const std::vector<const BRDF*>& brdfs() const { return brdfs_; };
    
    ~Material() {
	for (const BRDF* brdf : brdfs_) {
	    delete brdf;
	}
    }
};

class LambertMaterial : public Material {
public:
    LambertMaterial(const RGBColor& albedo);
};

class MicrofacetMaterial : public Material {
public:
    MicrofacetMaterial(const RGBColor& albedo, float specular_ratio, float roughness, float ior);
};

class Emission : public Material {
public:
    Emission(const RGBColor& irradiance);
};
