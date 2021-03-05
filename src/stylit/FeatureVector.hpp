#pragma once

#include "../Color.hpp"

struct FeatureVector {
    RGBColor full;
    RGBColor lde;
    RGBColor lse;
    RGBColor l_any_dde;
    RGBColor l_d_or_dd_e;

    const FeatureVector& operator+=(const FeatureVector& other);
    const FeatureVector& operator-=(const FeatureVector& other);
    const FeatureVector& operator*=(float t);
    const FeatureVector& operator/=(float t);
};

FeatureVector operator+(const FeatureVector& lhs, const FeatureVector& rhs);
FeatureVector operator-(const FeatureVector& lhs, const FeatureVector& rhs);
FeatureVector operator*(const FeatureVector& lhs, float t);
FeatureVector operator/(const FeatureVector& lhs, float t);

float dot(const FeatureVector& lhs, const FeatureVector& rhs);
float norm_squared(const FeatureVector& v);
float norm(const FeatureVector& v);
