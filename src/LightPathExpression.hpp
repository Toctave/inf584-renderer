#pragma once

#include "Material.hpp"
#include <vector>

SurfaceType surface_type_from_char(char c);
char surface_type_to_char(SurfaceType t);

class LightPathExpression {
private:
    std::vector<SurfaceType> surface_types_;

public:
    LightPathExpression();
    LightPathExpression(const std::string& bounces);

    SurfaceType operator[](size_t idx) const;
    size_t length() const;

    void push_back(const SurfaceType& surface_type);
    void pop_back();

    void reverse();
};

std::ostream& operator<<(std::ostream& out, const LightPathExpression& expr);

bool match(const LightPathExpression& pattern, const LightPathExpression& expr);
