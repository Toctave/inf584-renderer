#pragma once

#include "Ray.hpp"
#include "Vec.hpp"

class Camera {
private:
    Vec3 position_;
    Vec3 frame_[3];
    float aspect_ratio_;
    float depth_;

public:
    Camera();
    Camera(Vec3 eye, Vec3 target, Vec3 up, float fovy, float ar);
    Ray get_ray(Vec2 sample) const;

    const Vec3& position() const;
};
