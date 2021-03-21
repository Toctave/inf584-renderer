#pragma once

#include "../Vec.hpp"

template<typename T>
class DynaVec {
private:
    std::vector<T> co_;

public:
    DynaVec();
    DynaVec(size_t dim);

    const T& operator[](size_t i) const;
    T& operator[](size_t i);

    const DynaVec<T>& operator+=(const DynaVec<T>& other);
    const DynaVec<T>& operator-=(const DynaVec<T>& other);
    const DynaVec<T>& operator*=(const T& other);
    const DynaVec<T>& operator/=(const T& other);

    void normalize();
    DynaVec<T> normalized() const;

    template<size_t N>
    const DynaVec<T>& extend(const Vec<T, N>& v);

    const DynaVec<T>& extend(const DynaVec<T>& v);
    const DynaVec<T>& extend(const T& v);

    const T* data() const;

    size_t dim() const;
};

template<typename T>
DynaVec<T> operator+(const DynaVec<T>& lhs, const DynaVec<T>& rhs);

template<typename T>
DynaVec<T> operator-(const DynaVec<T>& lhs, const DynaVec<T>& rhs);

template<typename T>
DynaVec<T> operator-(const DynaVec<T>& lhs);

template<typename T>
DynaVec<T> operator*(const DynaVec<T>& lhs, const T& rhs);

template<typename T>
DynaVec<T> operator*(const T& lhs, const DynaVec<T>& rhs);

template<typename T>
DynaVec<T> operator/(const DynaVec<T>& lhs, const T& rhs);

template<typename T>
std::ostream& operator<<(std::ostream& out, const DynaVec<T>& v);

template<typename T>
T dot(const DynaVec<T>& lhs, const DynaVec<T>& rhs);

template<typename T>
T norm_squared(const DynaVec<T>& v);

template<typename T>
T norm(const DynaVec<T>& v);

#include "DynaVec.ipp"
