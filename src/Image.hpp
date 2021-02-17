#pragma once

#include <vector>
#include "Color.hpp"

class RGBImage {
private:
    std::vector<RGBColor> pixels_;
    size_t width_;
    size_t height_;

public:
    RGBImage(size_t width, size_t height);

    const RGBColor& operator()(size_t i, size_t j) const;
    RGBColor& operator()(size_t i, size_t j);

    void output_ppm(std::ostream& out) const;
};
