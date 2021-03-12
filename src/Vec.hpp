#pragma once

#include <cstddef>
#include <iostream>

template<typename T, size_t Rows, size_t Cols>
class Matrix;

template<typename T, size_t N>
class Vec;

template<typename T, size_t N>
Matrix<T, 1, N> row_matrix(const Vec<T, N>& v);

template<typename T, size_t N>
Matrix<T, N, 1> column_matrix(const Vec<T, N>& v);

template<typename T, size_t N>
class Vec {
private:
    T co_[N];

public:
    template<typename... Args, typename std::enable_if<sizeof...(Args) == N, int>::type = 0>
    Vec(Args&&... args);

    Vec(const T (&args)[N]);
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

    void normalize();
    Vec<T, N> normalized() const;

    friend Matrix<T, 1, N> row_matrix<>(const Vec<T, N>& v);
    friend Matrix<T, N, 1> column_matrix<>(const Vec<T, N>& v);
};

template<typename T, size_t N>
Vec<T, N> operator+(const Vec<T, N>& lhs, const Vec<T, N>& rhs);

template<typename T, size_t N>
Vec<T, N> operator-(const Vec<T, N>& lhs, const Vec<T, N>& rhs);

template<typename T, size_t N>
Vec<T, N> operator-(const Vec<T, N>& lhs);

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

template<typename T, size_t N>
T norm_squared(const Vec<T, N>& v);

template<typename T, size_t N>
T norm(const Vec<T, N>& v);

template<typename T, size_t N>
Vec<T, N> lerp(const Vec<T, N>& lhs, const Vec<T, N>& rhs, const T& ratio);

template<typename T>
Vec<T, 3> cross(const Vec<T, 3>& lhs, const Vec<T, 3>& rhs);

typedef Vec<float, 2> Vec2;
typedef Vec<float, 3> Vec3;
typedef Vec<float, 4> Vec4;
typedef Vec<size_t, 2> Vec2s;

#include "Vec.ipp"
