#include "Image.hpp"

#include <stdexcept>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

template<typename T>
Buffer2D<T> resized(const Buffer2D<T>& buffer, size_t new_rows, size_t new_columns, ResizeFilter filter) {
    Buffer2D<T> new_buffer(new_rows, new_columns);

    for (size_t row = 0; row < new_rows; row++) {
	for (size_t col = 0; col < new_columns; col++) {
	    if (filter == NEAREST_NEIGHBOUR) {
		size_t old_row = row * buffer.rows() / new_rows;
		size_t old_col = col * buffer.columns() / new_columns;

		new_buffer(row, col) = buffer(old_row, old_col);
	    } else {
		throw new std::invalid_argument("Unknown resize filter");
	    }
	}
    }

    return new_buffer;
}

RGBFilm::RGBFilm(size_t width, size_t height)
    : colors_(height, width), weights_(height, width) {
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

void RGBFilm::add_sample(const Vec2& pos, const RGBColor& color) {
    static const float firefly_threshold = 300.0f;
    static const float firefly_threshold_squared = firefly_threshold * firefly_threshold;
    static const float r = 1.3f;

    // discard samples carrying too much light
    // this biases the render, but leads to smoother results
    if (color.norm_squared() > firefly_threshold_squared) {
	return;
    }
    
    size_t colmin = static_cast<size_t>(std::floor(pos[0] + .5f - r));
    size_t colmax = static_cast<size_t>(std::ceil(pos[0] - 1.5f + r));

    size_t rowmin = static_cast<size_t>(std::floor(pos[1] + .5f - r));
    size_t rowmax = static_cast<size_t>(std::ceil(pos[1] - 1.5f + r));

    if (colmin >= height()) {
	colmin = 0;
    }
    if (colmax >= height()) {
	colmax = height() - 1;
    }
    
    if (rowmin >= width()) {
	rowmin = 0;
    }
    if (rowmax >= width()) {
	rowmax = width() - 1;
    }
    
    for (size_t row = rowmin; row <= rowmax; row++) {
	for (size_t col = colmin; col <= colmax; col++) {
	    Vec2 pixel_center(col + .5f, row + .5f);

	    Vec2 d = (pos - pixel_center) / r;
	    float weight = mitchell_filter(d) / (r * r);
	    
	    add_sample(row, col, color, weight);
	}
    }
}

Buffer2D<RGB8> RGBFilm::get_image() const {
    Buffer2D<RGB8> image(height(), width());

    for (size_t row = 0; row < height(); row++) {
	for (size_t col = 0; col < width(); col++) {
	    float w = weights_(row, col);
	    if (w == 0.0f) {
		image(row, col) = RGBColor(1, 0, 1).to_8bit();
	    } else {
		image(row, col) = RGBColor(colors_(row, col) / w).to_8bit();
	    }
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

size_t RGBFilm::width() const {
    return colors_.columns();
}

size_t RGBFilm::height() const {
    return colors_.rows();
}
