#pragma once

#include "Shape.hpp"
#include <vector>

class TriangleMesh : public Primitive {
private:
    std::vector<Vec3> vertices_;
    std::vector<size_t> indices_;
    
public:
    TriangleMesh(const std::string& obj_filepath);
    
    virtual float ray_intersect(const Ray& ray) const;
    virtual bool ray_intersect(const Ray& ray, Intersect& intersect) const;
    virtual Vec3 sample(float& pdf) const;
};
