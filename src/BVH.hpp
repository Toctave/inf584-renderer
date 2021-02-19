#pragma once

#include <vector>
#include "TriangleMesh.hpp"
#include "AABB.hpp"

class BVHNode {
private:
    AABB box_;
    union {
        const BVHNode* children_[2];
        std::vector<size_t> indices_;
    };
    bool is_leaf_;

    BVHNode& operator=(const BVHNode& other);
    
public:
    BVHNode();
    BVHNode(BVHNode&& other);
    ~BVHNode();
    
    BVHNode(const std::vector<size_t>::iterator& indices_begin,
            const std::vector<size_t>::iterator& indices_end,
            const TriangleMesh& mesh);
    BVHNode(const BVHNode* left, const BVHNode* right);
    static BVHNode from_mesh(const TriangleMesh& mesh);

    void print(size_t depth = 0) const;

    const BVHNode* left() const;
    const BVHNode* right() const;
    const std::vector<size_t>& indices() const;
    
    bool is_leaf() const;
    const AABB& box() const;
};


