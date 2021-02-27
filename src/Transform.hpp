#pragma once

#include "Vec.hpp"
#include "Matrix.hpp"
#include "Ray.hpp"

class Transform {
private:
    Matrix4 forwards_;
    Matrix4 backwards_;

public:
    Transform();
    Transform(Matrix4&& forwards, Matrix4&& backwards);
    Transform(const Matrix4& forwards, const Matrix4& backwards);

    static Transform translate(float x, float y, float z);
    static Transform scale(float sx, float sy, float sz);
    static Transform scale(float s);
    static Transform rotate(Vec3 axis, float angle);

    const Transform& operator*=(const Transform& other);

    Transform inverse() const;
    Transform transpose() const;

    Vec4 operator()(const Vec4& v) const;
};

Transform operator*(const Transform& lhs, const Transform& rhs);

Vec3 transform_point(const Transform& t, const Vec3& p); 
Vec3 transform_vector(const Transform& t, const Vec3& v);
Vec3 transform_normal(const Transform& t, const Vec3& n);

Ray transform_ray(const Transform& t, const Ray& r); 
