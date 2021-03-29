#include "Color.hpp"

#include <cmath>

RGBColor::RGBColor() : Vec3() {
}

RGBColor::RGBColor(float r, float g, float b) : Vec3({r, g, b}) {
}

RGBColor RGBColor::gray(float t) {
    return RGBColor(t, t, t);
}

RGBColor::RGBColor(const Vec3& v) : Vec3(v) {
}

RGBColor RGBColor::from_normal(const Vec3& normal) {
    return RGBColor(.5f * (normal + Vec3(1.0f)));
}

void clamp_to_01(float& val) {
    if (val < 0.0f) {
        val = 0.0f;
    } else if (val > 1.0f) {
        val = 1.0f;
    }
}

float linear_to_srgb(float lin) {
    if (lin <= .0031308) {
        return lin * 12.92;
    } else if (lin >= 1.0f) {
        return 1.0f;
    } else {
        float a = .055f;
        return (1 + a) * std::pow(lin, 1.0f / 2.4f) - a;
    }
}

float srgb_to_linear(float srgb) {
    if (srgb <= .04045f) {
        return srgb / 12.92f;
    } else if (srgb >= 1.0f) {
        return 1.0f;
    } else {
        float a = .055f;
        return std::pow((srgb + a) / (1.0f + a), 2.4f);
    }
}

RGB8 RGBColor::to_8bit() const {
    float r = (*this)[0];
    float g = (*this)[1];
    float b = (*this)[2];

    clamp_to_01(r);
    clamp_to_01(g);
    clamp_to_01(b);

    r = linear_to_srgb(r);
    g = linear_to_srgb(g);
    b = linear_to_srgb(b);

    if (std::isnan(r) || std::isnan(g) || std::isnan(b)) {
        std::cerr << "NAN in to_8bit\n";
    }

    return {
        static_cast<uint8_t>(r * 255.0f),
        static_cast<uint8_t>(g * 255.0f),
        static_cast<uint8_t>(b * 255.0f)
    };
}
    
const RGBColor& RGBColor::operator*=(const RGBColor& rhs) {
    for (size_t i = 0; i < 3; i++) {
        (*this)[i] *= rhs[i];
    }
    return *this;
}

RGBColor operator*(const RGBColor& lhs, const RGBColor& rhs) {
    RGBColor result = lhs;
    return result *= rhs;
}

RGBColor srgb_to_linear(const RGB8& rgb8) {
    RGBColor result(
	static_cast<float>(rgb8.r) / 255.0f,
	static_cast<float>(rgb8.g) / 255.0f,
	static_cast<float>(rgb8.b) / 255.0f
	);

    for (size_t i = 0; i < 3; i++) {
	result[i] = srgb_to_linear(result[i]);
    }

    return result;
}
