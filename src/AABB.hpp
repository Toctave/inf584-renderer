#pragma once

#include "Vec.hpp"
#include "Ray.hpp"

class AABB {
private:
    Vec3 min_;
    Vec3 max_;

public:
    AABB();
    AABB(const Vec3& min, const Vec3& max);
    
    const Vec3& min() const;
    const Vec3& max() const;

    bool ray_intersect(const Ray& ray) const;
    void include_point(const Vec3& p);

    void widen(float w);
};
