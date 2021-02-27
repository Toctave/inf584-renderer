#include "Matrix.hpp"

template<typename T, size_t Rows, size_t Cols>
template<typename... Args, typename std::enable_if<sizeof...(Args) == Rows * Cols, int>::type>
Matrix<T, Rows, Cols>::Matrix(Args&&... args) : co_{args...} {
}

template<typename T, size_t Rows, size_t Cols>
Matrix<T, Rows, Cols>::Matrix(const T (&args)[Rows * Cols]) {
    for (size_t i = 0; i < Rows * Cols; i++) {
	co_[i] = args[i];
    }
}

template<typename T, size_t Rows, size_t Cols>
Matrix<T, Rows, Cols>::Matrix() {
    for (size_t i = 0; i < Rows * Cols; i++) {
	co_[i] = 0;
    }
}

template<typename T, size_t N>
template<typename... Args, typename std::enable_if<sizeof...(Args) == N * N, int>::type>
SquareMatrix<T, N>::SquareMatrix(Args&&... args) : Matrix<T, N, N>(args...) {
}

template<typename T, size_t N>
SquareMatrix<T, N>::SquareMatrix(const T& lambda) {
    for (size_t i = 0; i < N; i++) {
	for (size_t j = 0; j < N; j++) {
	    (*this)(i, j) = (i == j) ? lambda : 0;
	}
    }
}

template<typename T, size_t N>
SquareMatrix<T, N>::SquareMatrix()
    : Matrix<T, N, N>() {
}

template<typename T, size_t N>
SquareMatrix<T, N>::SquareMatrix(Matrix<T, N, N>&& mat)
    : Matrix<T, N, N>(mat) {
}

template<typename T, size_t N>
void swap_rows(SquareMatrix<T, N>& mat, size_t i, size_t j) {
    for (size_t k = 0; k < N; k++) {
	T tmp = mat(i, k);
	mat(i, k) = mat(j, k);
	mat(j, k) = tmp;
    }
}

template<typename T, size_t N>
SquareMatrix<T, N> SquareMatrix<T, N>::inverse() const {
    SquareMatrix<T, N> inv(1);
    SquareMatrix<T, N> this_copy(*this);

    // Iterate over all rows
    for (size_t i = 0; i < N; i++) {
	// Use the largest possible pivot element
	size_t pivot_line = ~0ul;
	T best_pivot_abs = 0;
	for (size_t j = i; j < N; j++) {
	    T pivot_abs_candidate = std::abs(this_copy(j, i));
	    if (pivot_abs_candidate > best_pivot_abs) {
		pivot_line = j;
		best_pivot_abs = pivot_abs_candidate;
	    }
	}
	
	if (best_pivot_abs == 0) {
	    throw std::invalid_argument("Non-invertible matrix");
	}

	if (pivot_line != i) {
	    swap_rows(this_copy, pivot_line, i);
	    swap_rows(inv, pivot_line, i);
	}

	// for all rows other than this one
	for (size_t j = 0; j < N; j++) {
	    if (i == j) continue;
	    T ratio = this_copy(j, i) / this_copy(i, i);

	    for (size_t k = 0; k < N; k++) {
		this_copy(j, k) -= ratio * this_copy(i, k);
		inv(j, k) -= ratio * inv(i, k);
	    }
	}
    }

    // divide all rows by the diagonal element to get ones on the diagonal
    for (size_t i = 0; i < N; i++) {
	const T& u = this_copy(i, i);
	for (size_t j = 0; j < N; j++) {
	    inv(i, j) /= u;
	}
    }

    return inv;
}

template<typename T, size_t N>
SquareMatrix<T, N>::SquareMatrix(const T (&args)[N * N]) : Matrix<T, N, N>(args) {
}

template<typename T, size_t Rows, size_t Cols>
std::ostream& operator<<(std::ostream& stream, const Matrix<T, Rows, Cols>& m) {
    for (size_t i = 0; i < Rows; i++) {
	stream << "[ ";
	for (size_t j = 0; j < Cols; j++) {
	    stream << m(i, j) << " ";
	}
	stream << "]\n";
    }

    return stream;
}

template<typename T, size_t Rows, size_t Cols>
const T& Matrix<T, Rows, Cols>::operator()(size_t i, size_t j) const {
    assert(i < Rows);
    assert(j < Cols);
    return co_[i * Cols + j];
}

template<typename T, size_t Rows, size_t Cols>
T& Matrix<T, Rows, Cols>::operator()(size_t i, size_t j) {
    assert(i < Rows);
    assert(j < Cols);
    return co_[i * Cols + j];
}

template<typename T, size_t Rows, size_t Cols>
const Matrix<T, Rows, Cols>& Matrix<T, Rows, Cols>::operator+=(const Matrix<T, Rows, Cols>& other) {
    for (size_t i = 0; i < Rows; i++) {
	for (size_t j = 0; j < Cols; j++) {
	    (*this)(i, j) += other(i, j);
	}
    }
    return *this;
}

template<typename T, size_t Rows, size_t Cols>
const Matrix<T, Rows, Cols>& Matrix<T, Rows, Cols>::operator-=(const Matrix<T, Rows, Cols>& other) {
    for (size_t i = 0; i < Rows; i++) {
	for (size_t j = 0; j < Cols; j++) {
	    (*this)(i, j) -= other(i, j);
	}
    }
    return *this;
}
 
template<typename T, size_t Rows, size_t Cols>
const Matrix<T, Rows, Cols>& Matrix<T, Rows, Cols>::operator*=(const T& factor) {
    for (size_t i = 0; i < Rows; i++) {
	for (size_t j = 0; j < Cols; j++) {
	    (*this)(i, j) *= factor;
	}
    }
    return *this;
}

template<typename T, size_t Rows, size_t Cols>
const Matrix<T, Rows, Cols>& Matrix<T, Rows, Cols>::operator/=(const T& factor) {
    for (size_t i = 0; i < Rows; i++) {
	for (size_t j = 0; j < Cols; j++) {
	    (*this)(i, j) /= factor;
	}
    }
    return *this;
}

template<typename T, size_t Rows, size_t Cols>
const Matrix<T, Rows, Cols>& Matrix<T, Rows, Cols>::operator*=(const Matrix<T, Cols, Cols>& other) {
    (*this) = (*this) * other;
    return *this;
}

template<typename T, size_t M, size_t N, size_t L>
Matrix<T, M, L> operator*(const Matrix<T, M, N>& lhs, const Matrix<T, N, L>& rhs) {
    Matrix<T, M, L> result;
    
    for (size_t i = 0; i < M; i++) {
	for (size_t j = 0; j < L; j++) {
	    for (size_t k = 0; k < N; k++) {
		result(i, j) += lhs(i, k) * rhs(k, j);
	    }
	}
    }

    return result;
}

template<typename T, size_t Rows, size_t Cols>
Vec<T, Rows * Cols> Matrix<T, Rows, Cols>::flattened() const {
    return Vec<T, Rows * Cols>(co_);
}

template<typename T, size_t N>
SquareMatrix<T, N> SquareMatrix<T, N>::transpose() const {
    SquareMatrix<T, N> result;

    for (size_t i = 0; i < N; i++) {
	for (size_t j = 0; j < N; j++) {
	    result(j, i) = (*this)(i, j);
	}
    }
    
    return result;
}


template<typename T, size_t M, size_t N>
Vec<T, M> operator*(const Matrix<T, M, N>& lhs, const Vec<T, N>& rhs) {
    return (lhs * column_matrix(rhs)).flattened();
}

template<typename T, size_t N>
Matrix<T, 1, N> row_matrix(const Vec<T, N>& v) {
    return Matrix<T, 1, N>(v.co_);
}

template<typename T, size_t N>
Matrix<T, N, 1> column_matrix(const Vec<T, N>& v) {
    return Matrix<T, N, 1>(v.co_);
}
