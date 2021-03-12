#include "LightPathExpression.hpp"

SurfaceType surface_type_from_char(char c) {
    switch(c) {
    case 'E':
        return SurfaceType::EYE;
    case 'D':
        return SurfaceType::DIFFUSE;
    case 'S':
        return SurfaceType::SPECULAR;
    case 'L':
        return SurfaceType::LIGHT;
    case '*':
	return SurfaceType::REPEAT;
    case '.':
	return SurfaceType::ANY;
    default:
        throw std::invalid_argument("Character does not match a surface type.");
    }
}

char surface_type_to_char(SurfaceType type) {
    switch(type) {
    case SurfaceType::EYE:
        return 'E';
    case SurfaceType::DIFFUSE:
        return 'D';
    case SurfaceType::SPECULAR:
        return 'S';
    case SurfaceType::LIGHT:
        return 'L';
    case SurfaceType::ANY:
	return '.';
    case SurfaceType::REPEAT:
	return '*';
    default:
        return '?';
    }
}

LightPathExpression::LightPathExpression(const std::string& bounces)
    : surface_types_(bounces.length()) {
    for (size_t i = 0; i < bounces.length(); i++) {
	surface_types_[i] = surface_type_from_char(bounces[i]);
    }
}

SurfaceType LightPathExpression::operator[](size_t idx) const {
    return surface_types_[idx];
}

std::ostream& operator<<(std::ostream& out, const LightPathExpression& expr) {
    for (size_t i = 0; i < expr.length(); i++) {
	out << surface_type_to_char(expr[i]);
    }
    return out;
}

size_t LightPathExpression::length() const {
    return surface_types_.size();
}
