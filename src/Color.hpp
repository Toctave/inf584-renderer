#pragma once

#include "Vec.hpp"

struct RGB8 {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

class RGBColor : public Vec3 {
private:
    
public:
    RGBColor();
    RGBColor(float r, float g, float b);
    RGBColor(const Vec3& v);

    static RGBColor gray(float t);
    static RGBColor from_normal(const Vec3& normal);
    
    const RGBColor& operator*=(const RGBColor& other);

    RGB8 to_8bit() const;
};

RGBColor operator*(const RGBColor& lhs, const RGBColor& rhs);
