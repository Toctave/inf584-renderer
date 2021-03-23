#pragma once

#include <vector>
#include "Color.hpp"

enum ResizeFilter { NEAREST_NEIGHBOUR, BILINEAR };

template<typename T>
class Buffer2D {
private:
    size_t rows_;
    size_t columns_;
    std::vector<T> data_;

    size_t index(size_t row, size_t col) const { return row * columns_ + col; }

public:
    Buffer2D(size_t rows, size_t columns)
	: rows_(rows), columns_(columns), data_(rows * columns) {
    }

    const T& operator()(size_t row, size_t col) const { return data_[index(row, col)]; }
    T& operator()(size_t row, size_t col) { return data_[index(row, col)]; }
    
    const T& operator()(Vec2s p) const { return data_[index(p[0], p[1])]; }
    T& operator()(Vec2s p) { return data_[index(p[0], p[1])]; }

    const T* data() const { return data_.data(); }

    size_t rows() const { return rows_; }
    size_t columns() const { return columns_; }
};

template<typename T>
Buffer2D<T> resized(const Buffer2D<T>& buffer, size_t new_rows, size_t new_columns) {
    Buffer2D<T> new_buffer(new_rows, new_columns);

    for (size_t row = 0; row < new_rows; row++) {
	for (size_t col = 0; col < new_columns; col++) {
	    size_t old_row = row * buffer.rows() / new_rows;
	    size_t old_col = col * buffer.columns() / new_columns;
	    
	    new_buffer(row, col) = buffer(old_row, old_col);
	}
    }

    return new_buffer;
}

Buffer2D<RGB8> to_rgb8(const Buffer2D<RGBColor>& color);
Buffer2D<RGBColor> to_rgbcolor(const Buffer2D<RGB8>& color);

Buffer2D<RGB8> read_png(const std::string& filepath);

class RGBFilm {
private:
    Buffer2D<RGBColor> colors_;
    Buffer2D<float> weights_;
    float filter_radius_;

    void add_sample(size_t row, size_t col, const RGBColor& color, float weight);
    RGBColor get_color(size_t row, size_t col) const;
    
public:
    RGBFilm(size_t width, size_t height, float filter_radius=1.0f);

    void add_sample(const Vec2& pos, RGBColor color);

    Buffer2D<RGBColor> get_colors() const;
    Buffer2D<RGB8> get_image() const;
    
    size_t width() const;
    size_t height() const;
};

void write_png(const Buffer2D<RGB8>& colors, std::ostream& out);
void write_ppm(const Buffer2D<RGB8>& colors, std::ostream& out);

