#include "BVH.hpp"

#include <algorithm>

static const size_t bvh_leaf_threshold = 10;

BVHNode::BVHNode()
    : is_leaf_(true) {
}

BVHNode::BVHNode(BVHNode&& other) {
    if (other.is_leaf_) {
        indices_ = other.indices_;
    } else {
        children_[0] = other.children_[0];
        children_[1] = other.children_[1];
    }
    is_leaf_ = other.is_leaf_;
    box_ = other.box_;
}

BVHNode::BVHNode(const BVHNode* left, const BVHNode* right)
    : children_{left, right}, is_leaf_(false) {
        box_ = left->box_;
        box_.include_point(right->box_.min());
        box_.include_point(right->box_.max());
    box_.widen(1.0e-6f);
}

BVHNode::BVHNode(const std::vector<size_t>::iterator& indices_begin,
                 const std::vector<size_t>::iterator& indices_end,
                 const TriangleMesh& mesh)
    : indices_(indices_begin, indices_end), is_leaf_(true) {
    for (size_t idx : indices_) {
        for (size_t i = 0; i < 3; i++) {
            box_.include_point(mesh.triangle(idx).positions[i]);
        }
    }
    box_.widen(1.0e-6f);
}

BVHNode build_bvh(std::vector<size_t>::iterator indices_begin,
                  std::vector<size_t>::iterator indices_end,
                  const TriangleMesh& mesh,
                  const std::vector<Vec3>& centroids,
                  size_t depth) {
    size_t index_count = indices_end - indices_begin;
    if (index_count < bvh_leaf_threshold) {
        return BVHNode(indices_begin, indices_end, mesh);
    } else {
        size_t axis = depth % 3;
        std::sort(indices_begin,
                  indices_end,
                  [&centroids, &axis](size_t i, size_t j){
                      return centroids[i][axis] < centroids[j][axis];
                  });

        std::vector<size_t>::iterator indices_mid =
            indices_begin + index_count / 2;
        BVHNode* left =
            new BVHNode(build_bvh(indices_begin,
                                  indices_mid,
                                  mesh,
                                  centroids,
                                  depth + 1
                                  ));
        BVHNode* right =
            new BVHNode(build_bvh(indices_mid,
                                  indices_end,
                                  mesh,
                                  centroids,
                                  depth + 1
                                  ));
        return BVHNode(left, right);
    }
}

void indent(size_t depth) {
    for (size_t i = 0; i < depth; i++) {
        std::cout << "  ";
    }
}

void BVHNode::print(size_t depth) const {
    indent(depth);
    std::cout << box_.min() << " -> " << box_.max() << "\n";
    if (is_leaf_) {
        indent(depth + 1);
        for (size_t idx : indices_) {
            std::cout << idx << " ";
        }
        std::cout << "\n";
    } else {
        children_[0]->print(depth + 1);
        children_[1]->print(depth + 1);
    }
}

BVHNode BVHNode::from_mesh(const TriangleMesh& mesh) {
    std::vector<Vec3> centroids;
    centroids.reserve(mesh.triangle_count());
    for (size_t i = 0; i < mesh.triangle_count(); i++) {
        Vec3 centroid;
        for (size_t j = 0; j < 3; j++) {
            centroid += mesh.triangle(i).positions[j];
        }
        centroid /= 3.0f;
        centroids.push_back(centroid);
    }

    std::vector<size_t> indices(mesh.triangle_count());
    for (size_t i = 0; i < indices.size(); i++) {
        indices[i] = i;
    }

    BVHNode root = build_bvh(indices.begin(),
                             indices.end(),
                             mesh,
                             centroids,
                             0);

    return root;
}

BVHNode::~BVHNode() {
    if (is_leaf_) {
        indices_.~vector();
    } else {
        delete children_[0];
        delete children_[1];
    }
}

const BVHNode* BVHNode::left() const {
    return children_[0];
}

const BVHNode* BVHNode::right() const {
    return children_[1];
}

bool BVHNode::is_leaf() const {
    return is_leaf_;
}

const std::vector<size_t>& BVHNode::indices() const {
    return indices_;
}

const AABB& BVHNode::box() const {
    return box_;
}
