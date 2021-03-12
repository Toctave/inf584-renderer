#include "Transform.hpp"

Transform::Transform()
    : forwards_(1.0f), backwards_(1.0f) {
}

Transform::Transform(Matrix4&& forwards, Matrix4&& backwards)
    : forwards_(forwards), backwards_(backwards) {
}

Transform::Transform(const Matrix4& forwards, const Matrix4& backwards)
    : forwards_(forwards), backwards_(backwards) {
}

Transform Transform::translate(float x, float y, float z) {
    return Transform(
	Matrix4(1.0f, 0.0f, 0.0f, x,
		0.0f, 1.0f, 0.0f, y,
		0.0f, 0.0f, 1.0f, z,
		0.0f, 0.0f, 0.0f, 1.0f),
	Matrix4(1.0f, 0.0f, 0.0f, -x,
		0.0f, 1.0f, 0.0f, -y,
		0.0f, 0.0f, 1.0f, -z,
		0.0f, 0.0f, 0.0f, 1.0f)
	);
}

Transform Transform::translate(const Vec3& p) {
    return translate(p[0], p[1], p[2]);
}

Transform Transform::scale(float sx, float sy, float sz) {
    return Transform(
	Matrix4(sx, 0.0f, 0.0f, 0.0f,
		0.0f, sy, 0.0f, 0.0f,
		0.0f, 0.0f, sz, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f),
	Matrix4(1.0f / sx, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f / sy, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f / sz, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f)
	);
}

static Matrix4 rotation_matrix(Vec3 axis, float angle) {
    float uxx = axis[0] * axis[0];
    float uxy = axis[0] * axis[1];
    float uxz = axis[0] * axis[2];
    float uyy = axis[1] * axis[1];
    float uyz = axis[1] * axis[2];
    float uzz = axis[2] * axis[2];

    float ux = axis[0];
    float uy = axis[1];
    float uz = axis[2];

    float c = std::cos(angle);
    float omc = 1.0f - c;
    
    float s = std::sin(angle);

    return Matrix4(
	     uxx * omc + c, uxy * omc - uz * s, uxz * omc - uy * s, 0.0f,
	uxy * omc + uz * s,      uyy * omc + c, uyz * omc - ux * s, 0.0f,
	uxz * omc - uy * s, uyz * omc + ux * s,      uzz * omc + c, 0.0f,
	              0.0f,               0.0f,               0.0f, 1.0f
    );
}

Transform Transform::rotate(Vec3 axis, float angle) {
    return Transform(
	rotation_matrix(axis, angle),
	rotation_matrix(axis, -angle)
    );
}

Transform Transform::scale(float s) {
    return scale(s, s, s);
}

const Transform& Transform::operator*=(const Transform& other) {
    forwards_ = forwards_ * other.forwards_;
    backwards_ = other.backwards_ * backwards_;
    return *this;
}

Transform operator*(const Transform& lhs, const Transform& rhs) {
    Transform result = lhs;
    return (result *= rhs);
}

Vec4 Transform::operator()(const Vec4& v) const {
    return forwards_ * v;
}


Transform Transform::inverse() const {
    return Transform(backwards_, forwards_);
}

Transform Transform::transpose() const {
    return Transform(forwards_.transpose(), backwards_.transpose());
}

Vec3 transform_point(const Transform& t, const Vec3& p) {
    Vec4 p_homo(p[0], p[1], p[2], 1.0f);
    p_homo = t(p_homo);

    return Vec3(p_homo[0], p_homo[1], p_homo[2]) / p_homo[3];
}

Vec3 transform_vector(const Transform& t, const Vec3& v) {
    Vec4 v_homo(v[0], v[1], v[2], 0.0f);
    v_homo = t(v_homo);

    return Vec3(v_homo[0], v_homo[1], v_homo[2]);
}

Vec3 transform_normal(const Transform& t, const Vec3& n) {
    return transform_vector(t.inverse().transpose(), n);
}

Ray transform_ray(const Transform& t, const Ray& r) {
    Ray result = Ray(transform_point(t, r.o), transform_vector(t, r.d));
    result.tmax = r.tmax;

    return result;
}
