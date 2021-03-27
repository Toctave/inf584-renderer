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

LightPathExpression::LightPathExpression() {
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

void LightPathExpression::push_back(const SurfaceType& surface_type) {
    surface_types_.push_back(surface_type);
}

void LightPathExpression::pop_back() {
    surface_types_.pop_back();
}

void LightPathExpression::reverse() {
    size_t n = surface_types_.size();
    std::vector<SurfaceType> new_surface_types(n);

    for (size_t i = 0; i < n; i++) {
	new_surface_types[i] = surface_types_[n - 1 - i];
    }

    surface_types_ = new_surface_types;
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

static bool match(const LightPathExpression& pattern, const LightPathExpression& expr, size_t pat_offset, size_t expr_offset) {
    bool pat_finished = (pat_offset == pattern.length());
    bool expr_finished = (expr_offset == expr.length());    
    if (pat_finished || expr_finished) {
	return pat_finished && expr_finished;
    } 

    switch (pattern[pat_offset]) {
    case SurfaceType::ANY:
	return match(pattern, expr, pat_offset + 1, expr_offset + 1);
	
    case SurfaceType::REPEAT:
	return match(pattern, expr, pat_offset + 1, expr_offset)
	    || match(pattern, expr, pat_offset, expr_offset + 1);
	
    default:
	return (pattern[pat_offset] == expr[expr_offset])
	    && match(pattern, expr, pat_offset + 1, expr_offset + 1);
    }
}

bool match(const LightPathExpression& pattern, const LightPathExpression& expr) {
    return match(pattern, expr, 0, 0);
}
