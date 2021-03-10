#pragma once

#include "Shape.hpp"
#include <vector>

typedef Vec<size_t, 3> Vec3s;

class BVHNode;

struct Triangle {
    Vec3 positions[3];
    Vec3 normals[3];
};

class TriangleMesh : public Primitive {
private:
    std::vector<Triangle> triangles_;
    std::vector<float> triangle_areas_;
    std::vector<float> triangle_areas_cumsum_;
    
    float total_area_;
    
    const BVHNode* bvh_;

    TriangleMesh& operator=(const TriangleMesh& other);
    TriangleMesh(const TriangleMesh& other);
    
    void calculate_vertex_normals();
    void calculate_areas();
    
public:
    TriangleMesh(const std::string& obj_filepath);
    ~TriangleMesh();
    
    TriangleMesh(TriangleMesh&& other);
    
    virtual bool ray_intersect(const Ray& ray) const override;
    virtual bool ray_intersect(const Ray& ray, Intersect& intersect) const;
    virtual Vec3 sample(float& pdf) const;
    virtual float area() const override;

    size_t triangle_count() const;
    const Triangle& triangle(size_t i) const;
};
