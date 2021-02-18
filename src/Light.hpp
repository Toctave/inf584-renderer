#pragma once

#include "Color.hpp"
#include "Vec.hpp"
#include "Ray.hpp"

struct LightSample {
    Ray shadow_ray;
    RGBColor intensity;

    LightSample(const Ray& shadow_ray, const RGBColor& intensity)
        : shadow_ray(shadow_ray), intensity(intensity) {}
};

class Light {
public:
    virtual LightSample sample(const Vec3& point) const = 0;
};

class PointLight : public Light {
private:
    Vec3 position_;
    RGBColor intensity_;
    
public:
    PointLight(const Vec3& position, const RGBColor& color, float intensity = 1.0f);

    virtual LightSample sample(const Vec3& point) const;
};
