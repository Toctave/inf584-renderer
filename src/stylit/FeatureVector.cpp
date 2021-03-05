#include "FeatureVector.hpp"

const FeatureVector& FeatureVector::operator+=(const FeatureVector& other) {
    full += other.full;
    lde += other.lde;
    lse += other.lse;
    l_any_dde += other.l_any_dde;
    l_d_or_dd_e += other.l_d_or_dd_e;
    return *this;
}

const FeatureVector& FeatureVector::operator-=(const FeatureVector& other) {
    full -= other.full;
    lde -= other.lde;
    lse -= other.lse;
    l_any_dde -= other.l_any_dde;
    l_d_or_dd_e -= other.l_d_or_dd_e;
    return *this;
}

const FeatureVector& FeatureVector::operator*=(float t) {
    full = t * full;
    lde = t * lde;
    lse = t * lse;
    l_any_dde = t * l_any_dde;
    l_d_or_dd_e = t * l_d_or_dd_e;
    return *this;
}

const FeatureVector& FeatureVector::operator/=(float t) {
    full = full / t;
    lde = lde / t;
    lse = lse /  t;
    l_any_dde = l_any_dde / t;
    l_d_or_dd_e = l_d_or_dd_e / t;
    return *this;
}

FeatureVector operator+(const FeatureVector& lhs, const FeatureVector& rhs) {
    FeatureVector result = lhs;
    return (result += rhs);
}

FeatureVector operator-(const FeatureVector& lhs, const FeatureVector& rhs) {
    FeatureVector result = lhs;
    return (result -= rhs);
}

FeatureVector operator*(const FeatureVector& lhs, float t) {
    FeatureVector result = lhs;
    return (result *= t);
}

FeatureVector operator/(const FeatureVector& lhs, float t) {
    FeatureVector result = lhs;
    return (result /= t);
}

float dot(const FeatureVector& lhs, const FeatureVector& rhs) {
    return dot(lhs.full, rhs.full)
	+ dot(lhs.lde, rhs.lde)
	+ dot(lhs.lse, rhs.lse)
	+ dot(lhs.l_any_dde, rhs.l_any_dde)
	+ dot(lhs.l_d_or_dd_e, rhs.l_d_or_dd_e);
}

float norm_squared(const FeatureVector& v) {
    return dot(v, v);
}

float norm(const FeatureVector& v) {
    return std::sqrt(norm_squared(v));
}

