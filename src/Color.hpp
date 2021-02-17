#pragma once

#include "Vec.hpp"

struct RGB8 {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

class RGBColor : public Vec3 {
private:
    float norm_squared() const;
    float norm() const;
    
public:
    RGBColor();
    RGBColor(float r, float g, float b);
    RGBColor(float gray);
    
    const RGBColor& operator*=(const RGBColor& other);

    RGB8 to_8bit() const;
};

RGBColor operator*(const RGBColor& lhs, const RGBColor& rhs);
