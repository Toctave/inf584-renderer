#pragma once

#include <vector>

#include "Ray.hpp"
#include "Color.hpp"
#include "Intersect.hpp"
#include "Material.hpp"
#include "LightPathExpression.hpp"

class LightPath {
private:
    SurfaceType type_;
    Vec3 point_;
    
    RGBColor emitted_;
    
    std::vector<const LightPath*> tributaries_;
    std::vector<float> pdfs_;
    std::vector<RGBColor> brdfs_;
    std::vector<float> angle_cos_;
    RGBColor radiance_channel(const LightPathExpression& channel, int offset) const;

public:
    LightPath(const Vec3& eye); // eye
    LightPath(SurfaceType type, const Vec3& point); // diffuse/specular
    LightPath(const RGBColor& emitted, const Vec3& point); // light
    ~LightPath();

    void add_tributary(LightPath* path,
                       float pdf,
                       const RGBColor& brdf,
                       float angle_cos);
    RGBColor radiance() const;

    void set_emission(const RGBColor& emission);
    RGBColor radiance_channel(const LightPathExpression& channel) const;
};
