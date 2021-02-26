#pragma once

#include "Vec.hpp"
#include "Matrix.hpp"

Vec3 transform_point(const Matrix4& mat, const Vec3& v); 
Vec3 transform_vector(const Matrix4& mat, const Vec3& v);
