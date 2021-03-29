#include "EigenBuffer2DView.hpp"

size_t EigenBuffer2DView::idx(size_t row, size_t col) const {
    return row * columns_ + col;
}
    
EigenBuffer2DView::EigenBuffer2DView(float* data, size_t rows, size_t columns, size_t depth)
    : rows_(rows),
      columns_(columns),
      depth_(depth),
      map_(data, depth, rows * columns) {
}

Eigen::VectorXf EigenBuffer2DView::operator()(size_t row, size_t col) const {
    return map_.col(idx(row, col));
}

Eigen::VectorXf EigenBuffer2DView::operator()(Vec2s p) const {
    return (*this)(p[0], p[1]);
}

Eigen::ArrayXXf EigenBuffer2DView::pixel_block(size_t row0, size_t col0, size_t rows, size_t cols) const {
    Eigen::ArrayXXf result(rows * cols, depth_);

    for (size_t dr = 0; dr < rows; dr++) {
	for (size_t dc = 0; dc < cols; dc++) {
	    result.row(dr * cols + dc) = (*this)(row0 + dr, col0 + dc);
	}
    }

    return result;
}

