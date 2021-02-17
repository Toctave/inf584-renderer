#include "Vec.hpp"

#include <cassert>

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


