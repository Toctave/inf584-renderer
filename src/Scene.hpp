#pragma once

#include <vector>
#include "Shape.hpp"

class Scene {
private:
    std::vector<const Shape*> shapes_;

public:
    bool ray_intersect(Ray& ray, Intersect& itx) const;
    void add_shape(const Shape* shape);
};
