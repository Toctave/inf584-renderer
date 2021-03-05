#pragma once

template<typename T>
class MipmapPyramid {
private:
    std::vector<Buffer2D<T>> buffers_;

public:
    MipmapPyramid(const Buffer2D<T>& base, size_t levels, float ratio);
};

#include "MipmapPyramid.ipp"
