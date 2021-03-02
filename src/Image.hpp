#pragma once

#include <vector>
#include "Color.hpp"

enum ResizeFilter { NEAREST_NEIGHBOUR };

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

    const T* data() const { return data_.data(); }

    size_t rows() const { return rows_; }
    size_t columns() const { return columns_; }
};

template<typename T>
Buffer2D<T> resized(const Buffer2D<T>& buffer, size_t new_rows, size_t new_columns, ResizeFilter filter);

class RGBFilm {
private:
    Buffer2D<RGBColor> colors_;
    Buffer2D<float> weights_;

    void add_sample(size_t row, size_t col, const RGBColor& color, float weight);
public:
    RGBFilm(size_t width, size_t height);

    void add_sample(const Vec2& pos, const RGBColor& color);

    Buffer2D<RGB8> get_image() const;
    
    size_t width() const;
    size_t height() const;
};

void write_png(const Buffer2D<RGB8>& colors, std::ostream& out);
void write_ppm(const Buffer2D<RGB8>& colors, std::ostream& out);
