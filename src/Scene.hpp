#pragma once

#include <vector>
#include "Shape.hpp"
#include "Light.hpp"

class Scene {
private:
    std::vector<const Shape*> shapes_;
    std::vector<const Light*> lights_;

public:
    bool ray_intersect(Ray& ray, Intersect& itx) const;
    bool ray_intersect(const Ray& ray) const;
    
    void add_shape(const Shape* shape);
    void add_light(const Light* light);

    const std::vector<const Light*>& lights() const;
};
