#pragma once

#include "Material.hpp"
#include <vector>

SurfaceType surface_type_from_char(char c);
char surface_type_to_char(SurfaceType t);

class LightPathExpression {
private:
    std::vector<SurfaceType> surface_types_;

public:
    LightPathExpression(const std::string& bounces);

    SurfaceType operator[](size_t idx) const;
    size_t length() const;
};

std::ostream& operator<<(std::ostream& out, const LightPathExpression& expr);
