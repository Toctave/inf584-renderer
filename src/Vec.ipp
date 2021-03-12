#include "Vec.hpp"

#include <stdexcept>
#include <cmath>
#include <cassert>

template<typename T, size_t N>
bool has_nan(const Vec<T, N>& lhs) {
    for (size_t i = 0; i < N; i++) {
        if (std::isnan(lhs[i])) {
            return true;
        }
    }
    return false;
}

template<typename T, size_t N>
template<typename... Args, typename std::enable_if<sizeof...(Args) == N, int>::type>
Vec<T, N>::Vec(Args&&... args)
    : co_{args...} {
    static_assert(sizeof...(Args) == N, "You must provide N arguments.");

    assert(!has_nan(*this));
}

template<typename T, size_t N>
Vec<T, N>::Vec(const T& val) {
    for (size_t i = 0; i < N; i++) {
        co_[i] = val;
    }
    assert(!has_nan(*this));
}

template<typename T, size_t N>
Vec<T, N>::Vec(const T (&args)[N]) {
    for (size_t i = 0; i < N; i++) {
        co_[i] = args[i];
    }
    assert(!has_nan(*this));
}

template<typename T, size_t N>
Vec<T, N>::Vec() {
    for (size_t i = 0; i < N; i++) {
        co_[i] = {0};
    }
    assert(!has_nan(*this));
}

template<typename T, size_t N>
Vec<T, N>::Vec(const Vec<T, N>& other) {
    for (size_t i = 0; i < N; i++) {
        co_[i] = other.co_[i];
    }
    assert(!has_nan(*this));
}

template<typename T, size_t N>
const Vec<T, N>& Vec<T, N>::operator=(const Vec<T, N>& other) {
    for (size_t i = 0; i < N; i++) {
        co_[i] = other.co_[i];
    }
    assert(!has_nan(*this));
    return *this;
}

template<typename T, size_t N>
const T& Vec<T, N>::operator[](size_t i) const {
    return co_[i];
}

template<typename T, size_t N>
T& Vec<T, N>::operator[](size_t i) {
    return co_[i];
}

template<typename T, size_t N>
const Vec<T, N>& Vec<T, N>::operator+=(const Vec<T, N>& other) {
    for (size_t i = 0; i < N; i++) {
        co_[i] += other.co_[i];
    }
    assert(!has_nan(*this));
    return *this;
}

template<typename T, size_t N>
const Vec<T, N>& Vec<T, N>::operator-=(const Vec<T, N>& other) {
    for (size_t i = 0; i < N; i++) {
        co_[i] -= other.co_[i];
    }
    assert(!has_nan(*this));
    return *this;
}

template<typename T, size_t N>
const Vec<T, N>& Vec<T, N>::operator*=(const T& other) {
    for (size_t i = 0; i < N; i++) {
        co_[i] *= other;
    }
    assert(!has_nan(*this));
    return *this;
}

template<typename T, size_t N>
const Vec<T, N>& Vec<T, N>::operator/=(const T& other) {
    for (size_t i = 0; i < N; i++) {
        assert(!(other == 0 && co_[i] == 0));
        co_[i] /= other;
    }
    assert(!has_nan(*this));
    return *this;
}

template<typename T, size_t N>
Vec<T, N> operator+(const Vec<T, N>& lhs, const Vec<T, N>& rhs) {
    assert(!has_nan(lhs) && !has_nan(rhs));
    Vec<T, N> result = lhs;
    return result += rhs;
}

template<typename T, size_t N>
Vec<T, N> operator-(const Vec<T, N>& lhs, const Vec<T, N>& rhs) {
    assert(!has_nan(lhs) && !has_nan(rhs));
    Vec<T, N> result = lhs;
    return result -= rhs;
}

template<typename T, size_t N>
Vec<T, N> operator-(const Vec<T, N>& lhs) {
    assert(!has_nan(lhs));
    Vec<T, N> result;
    return result -= lhs;
}

template<typename T, size_t N>
Vec<T, N> operator*(const Vec<T, N>& lhs, const T& rhs) {
    assert(!has_nan(lhs) && !std::isnan(rhs));
    Vec<T, N> result = lhs;
    return result *= rhs;
}

template<typename T, size_t N>
Vec<T, N> operator*(const T& lhs, const Vec<T, N>& rhs) {
    assert(!std::isnan(lhs) && !has_nan(rhs));
    Vec<T, N> result = rhs;
    return result *= lhs;
}

template<typename T, size_t N>
Vec<T, N> operator/(const Vec<T, N>& lhs, const T& rhs) {
    assert(!has_nan(lhs) && !std::isnan(rhs));
    Vec<T, N> result = lhs;
    return result /= rhs;
}

template<typename T, size_t N>
std::ostream& operator<<(std::ostream& out, const Vec<T, N>& v) {
    out << "( ";
    for (size_t i = 0; i + 1 < N; i++) {
        out << v[i] << ", ";
    }
    out << v[N - 1] << " )";
    return out;
}

template<typename T, size_t N>
T dot(const Vec<T, N>& lhs, const Vec<T, N>& rhs) {
    assert(!has_nan(lhs) && !has_nan(rhs));
    T sum = {0};

    for (size_t i = 0; i < N; i++) {
        sum += lhs[i] * rhs[i];
    }

    return sum;
}

template<typename T, size_t N>
Vec<T, N> lerp(const Vec<T, N>& lhs, const Vec<T, N>& rhs, const T& ratio) {
    assert(!has_nan(lhs) && !has_nan(rhs));
    return lhs * (1 - ratio) + rhs * ratio;
}

template<typename T>
Vec<T, 3> cross(const Vec<T, 3>& lhs, const Vec<T, 3>& rhs) {
    assert(!has_nan(lhs) && !has_nan(rhs));
    return Vec<T, 3>({
        lhs[1] * rhs[2] - lhs[2] * rhs[1],
        lhs[2] * rhs[0] - lhs[0] * rhs[2],
        lhs[0] * rhs[1] - lhs[1] * rhs[0]
        });
}

template<typename T, size_t N>
T norm_squared(const Vec<T, N>& v) {
    assert(!has_nan(v));
    return dot(v, v);
}

template<typename T, size_t N>
T norm(const Vec<T, N>& v) {
    assert(!has_nan(v));
    return std::sqrt(norm_squared(v));
}

template<typename T, size_t N>
void Vec<T, N>::normalize() {
    *this /= norm(*this);
    assert(!has_nan(*this));
}

template<typename T, size_t N>
Vec<T, N> Vec<T, N>::normalized() const {
    Vec<T, N> result = *this;

    result.normalize();

    assert(!has_nan(result));
    return result;
}

