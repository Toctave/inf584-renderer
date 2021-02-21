#pragma once

#include "Shape.hpp"
#include <vector>

typedef Vec<size_t, 3> Vec3s;

class BVHNode;

class TriangleMesh : public Primitive {
private:
    std::vector<Vec3> vertices_;
    std::vector<Vec3s> triangles_;
    const BVHNode* bvh_;

    TriangleMesh& operator=(const TriangleMesh& other);
    
public:
    TriangleMesh(const std::string& obj_filepath);
    ~TriangleMesh();
    
    TriangleMesh(TriangleMesh&& other);
    
    virtual bool ray_intersect(const Ray& ray) const override;
    virtual bool ray_intersect(const Ray& ray, Intersect& intersect) const;
    virtual Vec3 sample(float& pdf) const;

    const std::vector<Vec3>& vertices() const;
    const std::vector<Vec3s>& triangles() const;
};
