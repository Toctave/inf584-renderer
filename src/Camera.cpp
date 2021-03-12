#include "Camera.hpp"

Camera::Camera() {
}

Camera::Camera(Vec3 eye, Vec3 target, Vec3 up, float fovy, float ar)
    : position_(eye), aspect_ratio_(ar) {
    frame_[2] = (eye - target).normalized();
    frame_[0] = cross(up, frame_[2]).normalized();
    frame_[1] = cross(frame_[2], frame_[0]);

    depth_ = std::cos(fovy * .5f) / std::sin(fovy * .5f);
}

Ray Camera::get_ray(Vec2 sample) const {
    Vec3 d = sample[0] * aspect_ratio_ * frame_[0]
        + sample[1] * frame_[1]
        - depth_ * frame_[2];
    d.normalize();
    return Ray(position_,
               d);
}

const Vec3& Camera::position() const {
    return position_;
}
    
