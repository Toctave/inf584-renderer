#include "Image.hpp"

#include <stdexcept>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

RGBImage::RGBImage(size_t width, size_t height)
    : width_(width), height_(height), pixels_(width_ * height_) {
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


void stbi_write_callback(void *context, void *data, int size) {
    std::ostream* out = static_cast<std::ostream*>(context);
    char* data_c = static_cast<char*>(data);

    for (int i = 0; i < size; i++) {
	*out << data_c[i];
    }
}

void RGBImage::output_png(std::ostream& out) const {
    std::vector<RGB8> pixels_8bit;
    pixels_8bit.reserve(width_ * height_);
    for (const auto& pixel : pixels_) {
        pixels_8bit.push_back(pixel.to_8bit());
    }

    stbi_write_png_to_func(stbi_write_callback,
			   &out,
			   width_,
			   height_,
			   3,
			   pixels_8bit.data(),
			   width_ * sizeof(RGB8));
}

size_t RGBImage::width() const {
    return width_;
}

size_t RGBImage::height() const {
    return height_;
}
