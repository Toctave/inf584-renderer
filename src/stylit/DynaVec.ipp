
template<typename T>
void same_dim(const DynaVec<T>& v1, const DynaVec<T>& v2) {
    if (v1.dim() != v2.dim()) {
	throw std::invalid_argument("Dimensions don't match");
    }
}

template<typename T>
DynaVec<T>::DynaVec() {
}

template<typename T>
DynaVec<T>::DynaVec(size_t dim)
    : co_(dim) {
}

template<typename T>
const T& DynaVec<T>::operator[](size_t i) const {
    return co_[i];
}

template<typename T>
T& DynaVec<T>::operator[](size_t i) {
    return co_[i];
}

template<typename T>
const DynaVec<T>& DynaVec<T>::operator+=(const DynaVec<T>& other) {
    same_dim(*this, other);
    
    for (size_t i = 0; i < co_.size(); i++) {
	co_[i] += other.co_[i];
    }
    return *this;
}

template<typename T>
const DynaVec<T>& DynaVec<T>::operator-=(const DynaVec<T>& other) {
    same_dim(*this, other);
    
    for (size_t i = 0; i < co_.size(); i++) {
	co_[i] -= other.co_[i];
    }
    return *this;
}

template<typename T>
const DynaVec<T>& DynaVec<T>::operator*=(const T& other) {
    for (size_t i = 0; i < co_.size(); i++) {
	co_[i] *= other;
    }
    return *this;
}

template<typename T>
const DynaVec<T>& DynaVec<T>::operator/=(const T& other) {
    for (size_t i = 0; i < co_.size(); i++) {
	co_[i] /= other;
    }
    return *this;
}

template<typename T>
void DynaVec<T>::normalize() {
    *this /= norm(*this);
}

template<typename T>
DynaVec<T> DynaVec<T>::normalized() const {
    DynaVec<T> result = *this;
    result.normalize();

    return result;
}

template<typename T>
size_t DynaVec<T>::dim() const {
    return co_.size();
}

template<typename T>
DynaVec<T> operator+(const DynaVec<T>& lhs, const DynaVec<T>& rhs) {
    DynaVec<T> result = lhs;
    return result += rhs;
}

template<typename T>
DynaVec<T> operator-(const DynaVec<T>& lhs, const DynaVec<T>& rhs) {
    DynaVec<T> result = lhs;
    return result += rhs;
}

template<typename T>
DynaVec<T> operator-(const DynaVec<T>& lhs) {
    DynaVec<T> result;
    return result -= lhs;
}

template<typename T>
DynaVec<T> operator*(const DynaVec<T>& lhs, const T& rhs) {
    DynaVec<T> result = lhs;
    return result *= rhs;
}

template<typename T>
DynaVec<T> operator*(const T& lhs, const DynaVec<T>& rhs) {
    DynaVec<T> result = rhs;
    return result *= lhs;
}

template<typename T>
DynaVec<T> operator/(const DynaVec<T>& lhs, const T& rhs) {
    DynaVec<T> result = lhs;
    return result /= rhs;
}

template<typename T>
std::ostream& operator<<(std::ostream& out, const DynaVec<T>& v) {
    out << "( ";
    for (size_t i = 0; i < v.dim(); i++) {
	out << v[i] << ", ";
    }
    out << ")";
}

template<typename T>
template<size_t N>
const DynaVec<T>& DynaVec<T>::extend(const Vec<T, N>& v) {
    co_.reserve(co_.size() + N);
    for (size_t i = 0; i < N; i++) {
	co_.push_back(v[i]);
    }
    return *this;
}

template<typename T>
const DynaVec<T>& DynaVec<T>::extend(const DynaVec<T>& v) {
    co_.reserve(co_.size() + v.dim());
    for (size_t i = 0; i < v.dim(); i++) {
	co_.push_back(v[i]);
    }
    return *this;
}

template<typename T>
const DynaVec<T>& DynaVec<T>::extend(const T& u) {
    co_.push_back(u);
    return *this;
}

template<typename T>
T dot(const DynaVec<T>& lhs, const DynaVec<T>& rhs) {
    same_dim(lhs, rhs);
    
    T sum = {0};
    for (size_t i = 0; i < lhs.dim(); i++) {
	sum += lhs[i] * rhs[i];
    }
    return sum;
}

template<typename T>
T norm_squared(const DynaVec<T>& v) {
    return dot(v, v);
}

template<typename T>
T norm(const DynaVec<T>& v) {
    return std::sqrt(v);
}


