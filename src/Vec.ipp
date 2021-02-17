#include "Vec.hpp"

#include <stdexcept>

template<typename T, size_t N>
Vec<T, N>::Vec(const T(&args)[N]) {
    for (size_t i = 0; i < N; i++) {
        co_[i] = args[i];
    }
}

template<typename T, size_t N>
Vec<T, N>::Vec(const T& val) {
    for (size_t i = 0; i < N; i++) {
        co_[i] = val;
    }
}

template<typename T, size_t N>
Vec<T, N>::Vec() {
    for (size_t i = 0; i < N; i++) {
        co_[i] = {0};
    }
}

template<typename T, size_t N>
Vec<T, N>::Vec(const Vec<T, N>& other) {
    for (size_t i = 0; i < N; i++) {
        co_[i] = other.co_[i];
    }
}

template<typename T, size_t N>
const Vec<T, N>& Vec<T, N>::operator=(const Vec<T, N>& other) {
    for (size_t i = 0; i < N; i++) {
        co_[i] = other.co_[i];
    }
    return *this;
}

template<typename T, size_t N>
const T& Vec<T, N>::operator[](size_t i) const {
    if (i >= N) {
        throw std::out_of_range("Out of range Vec component access");
    }
    return co_[i];
}

template<typename T, size_t N>
T& Vec<T, N>::operator[](size_t i) {
    if (i >= N) {
        throw std::out_of_range("Out of range Vec component access");
    }
    return co_[i];
}

template<typename T, size_t N>
const Vec<T, N>& Vec<T, N>::operator+=(const Vec<T, N>& other) {
    for (size_t i = 0; i < N; i++) {
        co_[i] += other.co_[i];
    }
    return *this;
}

template<typename T, size_t N>
const Vec<T, N>& Vec<T, N>::operator-=(const Vec<T, N>& other) {
    for (size_t i = 0; i < N; i++) {
        co_[i] -= other.co_[i];
    }
    return *this;
}

template<typename T, size_t N>
const Vec<T, N>& Vec<T, N>::operator*=(const T& other) {
    for (size_t i = 0; i < N; i++) {
        co_[i] *= other;
    }
    return *this;
}

template<typename T, size_t N>
const Vec<T, N>& Vec<T, N>::operator/=(const T& other) {
    for (size_t i = 0; i < N; i++) {
        co_[i] /= other;
    }
    return *this;
}

template<typename T, size_t N>
Vec<T, N> operator+(const Vec<T, N>& lhs, const Vec<T, N>& rhs) {
    Vec<T, N> result = lhs;
    return result += rhs;
}

template<typename T, size_t N>
Vec<T, N> operator-(const Vec<T, N>& lhs, const Vec<T, N>& rhs) {
    Vec<T, N> result = lhs;
    return result -= rhs;
}

template<typename T, size_t N>
Vec<T, N> operator*(const Vec<T, N>& lhs, const T& rhs) {
    Vec<T, N> result = lhs;
    return result *= rhs;
}

template<typename T, size_t N>
Vec<T, N> operator*(const T& lhs, const Vec<T, N>& rhs) {
    Vec<T, N> result = rhs;
    return result *= lhs;
}

template<typename T, size_t N>
Vec<T, N> operator/(const Vec<T, N>& lhs, const T& rhs) {
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
    T sum = {0};

    for (size_t i = 0; i < N; i++) {
        sum += lhs[i] * rhs[i];
    }

    return sum;
}

template<typename T, size_t N>
Vec<T, N> lerp(const Vec<T, N>& lhs, const Vec<T, N>& rhs, const T& ratio) {
    return lhs * (1 - ratio) + rhs * ratio;
}

template<typename T>
Vec<T, 3> cross(const Vec<T, 3>& lhs, const Vec<T, 3>& rhs) {
    return Vec<T, 3>(
        lhs[1] * rhs[2] - lhs[2] * rhs[1],
        lhs[2] * rhs[0] - lhs[0] * rhs[2],
        lhs[0] * rhs[1] - lhs[1] * rhs[0]
    );
}

