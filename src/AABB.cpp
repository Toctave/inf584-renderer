#include "AABB.hpp"
#include "BVH.hpp"

AABB::AABB()
    : min_({INFTY, INFTY, INFTY}),
      max_({-INFTY, -INFTY, -INFTY}) {
}

AABB::AABB(const Vec3& min, const Vec3& max)
    : min_(min), max_(max) {
}
    
const Vec3& AABB::min() const {
    return min_;
}

const Vec3& AABB::max() const {
    return max_;
}

bool AABB::ray_intersect(const Ray& ray) const {
    float tmin = 0.0f;
    float tmax = ray.tmax;
	for (int a = 0; a < 3; ++a) {
		float invD = 1.0f / ray.d[a];
		float t0 = (min_[a] - ray.o[a]) * invD;
		float t1 = (max_[a] - ray.o[a]) * invD;
		if (invD < 0.0f) {
			float temp = t1;
			t1 = t0;
			t0 = temp;
		}

		tmin = t0 > tmin ? t0 : tmin;
		tmax = t1 < tmax ? t1 : tmax;

		if (tmax <= tmin)
			return false;
	}
    return true;
}

void AABB::include_point(const Vec3& p) {
    for (size_t i = 0; i < 3; i++) {
        if (p[i] > max_[i]) {
            max_[i] = p[i];
        }
        if (p[i] < min_[i]) {
            min_[i] = p[i];
        }
    }
}
