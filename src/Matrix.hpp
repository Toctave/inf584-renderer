#pragma once

template<typename T, size_t Rows, size_t Cols>
class Matrix {
private:
    T co_[Rows * Cols];

public:
    template<typename... Args, typename std::enable_if<sizeof...(Args) == Rows * Cols, int>::type = 0>
    Matrix(Args&&... args);
    
    Matrix(const T (&args)[Rows * Cols]);
    Matrix();

    const T& operator()(size_t i, size_t j) const;
    T& operator()(size_t i, size_t j);
    
    const Matrix<T, Rows, Cols>& operator+=(const Matrix<T, Rows, Cols>& other);
    const Matrix<T, Rows, Cols>& operator-=(const Matrix<T, Rows, Cols>& other);
    const Matrix<T, Rows, Cols>& operator*=(const T& factor);
    const Matrix<T, Rows, Cols>& operator/=(const T& factor);

    const Matrix<T, Rows, Cols>& operator*=(const Matrix<T, Cols, Cols>& other);

    Vec<T, Rows * Cols> flattened() const;
};

template<typename T, size_t N>
class SquareMatrix : public Matrix<T, N, N> {
public:
    template<typename... Args, typename std::enable_if<sizeof...(Args) == N * N, int>::type = 0>
    SquareMatrix(Args&&... args);
    
    SquareMatrix(const T& lambda);
    SquareMatrix(const T (&args)[N * N]);

    SquareMatrix<T, N> inverse() const;
};

template<typename T, size_t Rows, size_t Cols>
std::ostream& operator<<(std::ostream& stream, const Matrix<T, Rows, Cols>& m);

template<typename T, size_t N>
Matrix<T, 1, N> row_matrix(const Vec<T, N>& v);

template<typename T, size_t N>
Matrix<T, N, 1> column_matrix(const Vec<T, N>& v);

template<typename T, size_t M, size_t N, size_t L>
Matrix<T, M, L> operator*(const Matrix<T, M, N>& lhs, const Matrix<T, N, L>& rhs);

template<typename T, size_t M, size_t N>
Vec<T, M> operator*(const Matrix<T, M, N>& lhs, const Vec<T, N>& rhs);

typedef SquareMatrix<float, 2> Matrix2;
typedef SquareMatrix<float, 3> Matrix3;
typedef SquareMatrix<float, 4> Matrix4;

#include "Matrix.ipp"

