#include <Eigen/Core>

#include "../Image.hpp"

template<typename T>
class EigenArray2D {
    using InternalArray = Eigen::Array<T, Eigen::Dynamic, Eigen::Dynamic>;
    using ColBlock = Eigen::Block<InternalArray, Eigen::Dynamic, 1, true>;
    using ConstColBlock = const Eigen::Block<const InternalArray, Eigen::Dynamic, 1, true>;
    
private:
    size_t rows_;
    size_t columns_;
    
    InternalArray data_;

    size_t idx(size_t row, size_t col) const {
	return row * columns_ + col;
    }
    
public:
    EigenArray2D()
	: rows_(0),
	  columns_(0) {
    }
    
    EigenArray2D(size_t rows, size_t columns, size_t depth)
	: rows_(rows),
	  columns_(columns),
	  data_(depth, rows * columns) {
    }

    ColBlock
    operator()(size_t row, size_t col) {
	return data_.col(idx(row, col));
    }
    
    ConstColBlock
    operator()(size_t row, size_t col) const {
	return data_.col(idx(row, col));
    }
    
    ColBlock
    operator()(Vec2s p) {
    	return (*this)(p[0], p[1]);
    }

    ConstColBlock
    operator()(Vec2s p) const {
    	return (*this)(p[0], p[1]);
    }

    size_t columns() const {
	return columns_;
    }
    
    size_t rows() const {
	return rows_;
    }
    
    size_t depth() const {
	return data_.rows();
    }

    InternalArray
    block(size_t row0, size_t col0, size_t rows, size_t cols) const {
	InternalArray result(rows * cols, depth());

	for (size_t dr = 0; dr < rows; dr++) {
	    for (size_t dc = 0; dc < cols; dc++) {
		result.row(dr * cols + dc) = (*this)(row0 + dr, col0 + dc);
	    }
	}

	return result;
    }
};

