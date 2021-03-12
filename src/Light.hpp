#pragma once

#include "Color.hpp"
#include "Vec.hpp"
#include "Ray.hpp"
#include "Shape.hpp"

struct LightSample {
    Ray shadow_ray;
    RGBColor intensity;
    float pdf;

    LightSample(const Ray& shadow_ray, const RGBColor& intensity, float pdf)
        : shadow_ray(shadow_ray), intensity(intensity), pdf(pdf) {}
};

class Light {
public:
    virtual LightSample sample(const Vec3& point) const = 0;
    virtual bool is_shape(const Shape* shape) const = 0;
    virtual ~Light();
};

class PointLight : public Light {
private:
    Vec3 position_;
    RGBColor intensity_;
    
public:
    PointLight(const Vec3& position, const RGBColor& color, float intensity = 1.0f);

    virtual LightSample sample(const Vec3& point) const;
    virtual bool is_shape(const Shape* shape) const;
};

class AreaLight : public Light {
private:
    const Shape* shape_;

public:
    AreaLight(const Shape* shape);
    virtual LightSample sample(const Vec3& point) const;
    virtual bool is_shape(const Shape* shape) const;
};
