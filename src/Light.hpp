#pragma once

#include "Color.hpp"
#include "Vec.hpp"

struct LightSample {
    Vec3 wi;
    RGBColor intensity;
};

class Light {
    virtual LightSample sample(const Vec3& point) const = 0;
};

class PointLight : Light {
private:
    Vec3 position_;
    RGBColor intensity_;
    
public:
    PointLight(const Vec3& position, const RGBColor& color, float intensity = 1.0f);

    virtual LightSample sample(const Vec3& point) const;
};
