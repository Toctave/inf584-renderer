#include "Image.hpp"

#include <stdexcept>

RGBImage::RGBImage(size_t width, size_t height)
    : width_(width), height_(height), pixels_(width * height) {
}

const RGBColor& RGBImage::operator()(size_t i, size_t j) const {
    if (i >= width_ || j >= height_) {
        throw std::out_of_range("Out of range pixel access");
    }
    return pixels_[j * width_ + i];
}

RGBColor& RGBImage::operator()(size_t i, size_t j) {
    if (i >= width_ || j >= height_) {
        throw std::out_of_range("Out of range pixel access");
    }
    return pixels_[j * width_ + i];
}

void RGBImage::output_ppm(std::ostream& out) const {
    out << "P3\n"
        << width_ << " " << height_ << "\n"
        << "255\n";
    
    for (const auto& pixel : pixels_) {
        RGB8 p8 = pixel.to_8bit();
        out << +p8.r << " " << +p8.g << " " << +p8.b << " ";
    }
    out << "\n";
}
