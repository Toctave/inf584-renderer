#pragma once

#include <cstddef>
#include <iostream>

template<typename T, size_t N>
class Vec {
private:
    T co_[N];

public:
    Vec(const T(&args)[N]);
    Vec(const T& val);
    Vec();
    Vec(const Vec<T, N>& other);

    const Vec<T, N>& operator=(const Vec<T, N>& other);

    const T& operator[](size_t i) const;
    T& operator[](size_t i);

    const Vec<T, N>& operator+=(const Vec<T, N>& other);
    const Vec<T, N>& operator-=(const Vec<T, N>& other);
    const Vec<T, N>& operator*=(const T& other);
    const Vec<T, N>& operator/=(const T& other);

    T norm_squared() const;
    T norm() const;
};

template<typename T, size_t N>
Vec<T, N> operator+(const Vec<T, N>& lhs, const Vec<T, N>& rhs);

template<typename T, size_t N>
Vec<T, N> operator-(const Vec<T, N>& lhs, const Vec<T, N>& rhs);

template<typename T, size_t N>
Vec<T, N> operator*(const Vec<T, N>& lhs, const T& rhs);

template<typename T, size_t N>
Vec<T, N> operator*(const T& lhs, const Vec<T, N>& rhs);

template<typename T, size_t N>
Vec<T, N> operator/(const Vec<T, N>& lhs, const T& rhs);

template<typename T, size_t N>
std::ostream& operator<<(std::ostream& out, const Vec<T, N>& v);

template<typename T, size_t N>
T dot(const Vec<T, N>& lhs, const Vec<T, N>& rhs);

template<typename T>
Vec<T, 3> cross(const Vec<T, 3>& lhs, const Vec<T, 3>& rhs);

typedef Vec<float, 2> Vec2;
typedef Vec<float, 3> Vec3;
typedef Vec<float, 4> Vec4;

#include "Vec.ipp"
