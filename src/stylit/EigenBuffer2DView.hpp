#include <Eigen/Core>

#include "../Image.hpp"

class EigenBuffer2DView {
private:
    size_t rows_;
    size_t columns_;
    size_t depth_;

    Eigen::Map<Eigen::ArrayXXf> map_;

    size_t idx(size_t row, size_t col) const;
    
public:
    EigenBuffer2DView(float* data, size_t rows, size_t columns, size_t depth);

    Eigen::VectorXf operator()(size_t row, size_t col) const;
    // const Eigen::VectorXf& operator()(size_t row, size_t col) const;

    Eigen::VectorXf operator()(Vec2s p) const;
    // const Eigen::VectorXf& operator()(Vec2s p) const;

    Eigen::ArrayXXf pixel_block(size_t row0, size_t col0, size_t rows, size_t cols) const;
};

template<typename T>
EigenBuffer2DView as_eigen(Buffer2D<T>& buffer, size_t depth) {
    return EigenBuffer2DView((float*)buffer.data(), buffer.rows(), buffer.columns(), depth);
}

template<typename T>
const EigenBuffer2DView as_eigen(const Buffer2D<T>& buffer, size_t depth) {
    return EigenBuffer2DView((float*)buffer.data(), buffer.rows(), buffer.columns(), depth);
}
