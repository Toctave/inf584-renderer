#include "transform.hpp"

Vec3 transform_point(const Matrix4& mat, const Vec3& v) {
    Vec4 v_homo(v[0], v[1], v[2], 1.0f);
    v_homo = mat * v_homo;

    return Vec3(v_homo[0], v_homo[1], v_homo[2]) / v_homo[3];
}

Vec3 transform_vector(const Matrix4& mat, const Vec3& v) {
    Vec4 v_homo(v[0], v[1], v[2], 0.0f);
    v_homo = mat * v_homo;

    return Vec3(v_homo[0], v_homo[1], v_homo[2]);
}

