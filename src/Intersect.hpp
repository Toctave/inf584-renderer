#pragma once

#include "Ray.hpp"
#include <limits>
#include <cassert>

class Shape;
class Material;

struct Intersect {
    float t;

    Vec3 point;
    Vec3 wo;
    
    const Shape* shape;
    const Material* material;
    
    Vec3 normal;
    Vec3 local_x;
    Vec3 local_y;
    // Vec2 uv;

    Intersect()
        : t(INFTY),
          shape(nullptr),
          material(nullptr) {
    }

    void setup_local_basis() {
	local_y = cross(wo, normal);
	if (norm(local_y) < EPSILON) {
	    local_y = cross(wo + Vec3(1.0f, 0.0f, 0.0f),
				  normal);
	}
	local_y.normalize();
	local_x = cross(normal, local_y);
    }
};
