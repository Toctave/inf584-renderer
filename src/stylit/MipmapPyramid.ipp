template<typename T>
MipmapPyramid<T>::MipmapPyramid(const Buffer2D<T>& base, size_t levels, float ratio) {
    buffers_.reserve(levels);
    buffers_.push_back(base);

    size_t rows = base.rows();
    size_t columns = base.columns();

    for (size_t i = 1; i < levels; i++) {
	rows = static_cast<size_t>(rows * ratio);
	columns = static_cast<size_t>(columns * ratio);

	buffers_.push_back(resized(buffers_.back(), rows, columns));
    }
}

