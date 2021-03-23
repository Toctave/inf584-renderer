#include "Image.hpp"

#include <stdexcept>

#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#pragma GCC diagnostic pop

RGBFilm::RGBFilm(size_t width, size_t height, float filter_radius)
    : colors_(height, width), weights_(height, width), filter_radius_(filter_radius) {
}

Buffer2D<RGB8> to_rgb8(const Buffer2D<RGBColor>& color) {
    Buffer2D<RGB8> img(color.rows(), color.columns());
    
    for (size_t row = 0; row < color.rows(); row++) {
	for (size_t col = 0; col < color.columns(); col++) {
	    img(row, col) = color(row, col).to_8bit();
	}
    }

    return img;
}

Buffer2D<RGBColor> to_rgbcolor(const Buffer2D<RGB8>& color) {
    Buffer2D<RGBColor> img(color.rows(), color.columns());
    
    for (size_t row = 0; row < color.rows(); row++) {
	for (size_t col = 0; col < color.columns(); col++) {
	    img(row, col) = srgb_to_linear(color(row, col));
	}
    }

    return img;
}

static float mitchell_1d(float x) {
    float B = 1.0f / 3.0f;
    float C = 1.0f / 3.0f;
    
    x = std::abs(2 * x);
    if (x > 1)
        return ((-B - 6*C) * x*x*x + (6*B + 30*C) * x*x +
                (-12*B - 48*C) * x + (8*B + 24*C)) * (1.f/6.f);
    else
        return ((12 - 9*B - 6*C) * x*x*x + 
                (-18 + 12*B + 6*C) * x*x +
                (6 - 2*B)) * (1.f/6.f);
}

static float mitchell_filter(Vec2 u) {
    return mitchell_1d(u[0]) * mitchell_1d(u[1]);
}

void RGBFilm::add_sample(size_t row, size_t col, const RGBColor& color, float weight) {
    weights_(row, col) += weight;
    colors_(row, col) += color * weight;
}

void RGBFilm::add_sample(const Vec2& pos, RGBColor color) {
    static const float firefly_threshold = 300.0f;
    static const float firefly_threshold_squared = firefly_threshold * firefly_threshold;

    RGBColor sample_color = color;
    
    // clamp samples carrying too much light
    // this biases the render, but leads to smoother results
    if (norm_squared(sample_color) > firefly_threshold_squared) {
	sample_color = firefly_threshold * sample_color.normalized();
    }
    
    size_t colmin = static_cast<size_t>(std::floor(pos[0] + .5f - filter_radius_));
    size_t colmax = static_cast<size_t>(std::ceil(pos[0] - 1.5f + filter_radius_));

    size_t rowmin = static_cast<size_t>(std::floor(pos[1] + .5f - filter_radius_));
    size_t rowmax = static_cast<size_t>(std::ceil(pos[1] - 1.5f + filter_radius_));

    if (colmin >= width()) {
    	colmin = 0;
    }
    if (colmax >= width()) {
    	colmax = width() - 1;
    }
    
    if (rowmin >= height()) {
    	rowmin = 0;
    }
    if (rowmax >= height()) {
    	rowmax = height() - 1;
    }
    
    for (size_t row = rowmin; row <= rowmax; row++) {
	for (size_t col = colmin; col <= colmax; col++) {
	    Vec2 pixel_center(col + .5f, row + .5f);

	    Vec2 d = (pos - pixel_center) / filter_radius_;
	    float weight = mitchell_filter(d) / (filter_radius_ * filter_radius_);
	    
	    add_sample(row, col, sample_color, weight);
	}
    }
}

RGBColor RGBFilm::get_color(size_t row, size_t col) const {
    float w = weights_(row, col);
    if (w == 0.0f) {
	return RGBColor(0, 0, 0);
    } else {
	return colors_(row, col) / w;
    }
}

Buffer2D<RGB8> RGBFilm::get_image() const {
    Buffer2D<RGB8> image(height(), width());

    for (size_t row = 0; row < height(); row++) {
	for (size_t col = 0; col < width(); col++) {
	    image(row, col) = get_color(row, col).to_8bit();
	}
    }

    return image;
}

Buffer2D<RGBColor> RGBFilm::get_colors() const {
    Buffer2D<RGBColor> image(height(), width());

    for (size_t row = 0; row < height(); row++) {
	for (size_t col = 0; col < width(); col++) {
	    image(row, col) = get_color(row, col);
	}
    }

    return image;
}

void write_ppm(const Buffer2D<RGB8>& colors, std::ostream& out) {
    out << "P3\n"
        << colors.columns() << " " << colors.rows() << "\n"
        << "255\n";
    
    for (size_t row = 0; row < colors.rows(); row++) {
	for (size_t col = 0; col < colors.columns(); col++) {
	    RGB8 p8 = colors(row, col);
	    out << +p8.r << " " << +p8.g << " " << +p8.b << " ";
	}
    }
    out << "\n";
}

static void stbi_write_callback(void *context, void *data, int size) {
    std::ostream* out = static_cast<std::ostream*>(context);
    char* data_c = static_cast<char*>(data);

    for (int i = 0; i < size; i++) {
	*out << data_c[i];
    }
}

void write_png(const Buffer2D<RGB8>& colors, std::ostream& out) {
    stbi_write_png_to_func(stbi_write_callback,
			   &out,
			   colors.columns(),
			   colors.rows(),
			   3,
			   colors.data(),
			   colors.columns() * sizeof(RGB8));
}

Buffer2D<RGB8> read_png(const std::string& filepath) {
    int x, y;
    unsigned char* data = stbi_load(filepath.c_str(), &x, &y, nullptr, 3);

    Buffer2D<RGB8> result(x, y);

    memcpy(&result(0, 0), data, x * y * sizeof(RGB8));

    stbi_image_free(data);

    return result;
}


size_t RGBFilm::width() const {
    return colors_.columns();
}

size_t RGBFilm::height() const {
    return colors_.rows();
}
