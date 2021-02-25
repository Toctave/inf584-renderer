#pragma once

#include <vector>
#include "Color.hpp"

class RGBImage {
private:
    size_t width_;
    size_t height_;
    std::vector<RGBColor> pixels_;

public:
    RGBImage(size_t width, size_t height);
    RGBImage(size_t width, size_t height, const RGBColor& background);

    const RGBColor& operator()(size_t i, size_t j) const;
    RGBColor& operator()(size_t i, size_t j);

    void output_ppm(std::ostream& out) const;
    void output_png(std::ostream& out) const;

    size_t width() const;
    size_t height() const;
};
