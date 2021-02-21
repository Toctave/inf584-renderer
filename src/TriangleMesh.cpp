#include "TriangleMesh.hpp"
#include "BVH.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.hpp"

bool triangle_ray_intersect(const Triangle& triangle, const Ray& ray) {
    Vec3 edge1 = *triangle.positions[1] - *triangle.positions[0];
    Vec3 edge2 = *triangle.positions[2] - *triangle.positions[0];
    Vec3 h = cross(ray.d, edge2);
    float a = dot(edge1, h);
    if (a > -EPSILON && a < EPSILON)
        return false;    // ray parallel to the triangle

    float f = 1.0f/a;
    Vec3 s = ray.o - *triangle.positions[0];
    float u = f * dot(s, h);
    if (u < 0.0f || u > 1.0f)
        return false;
    Vec3 q = cross(s, edge1);
    float v = f * dot(ray.d, q);
    if (v < 0.0f || u + v > 1.0f)
        return false;

    // On calcule t pour savoir ou le point d'intersection se situe sur la ligne.
    float t = f * dot(edge2, q);
    if (t > 0.0f && t < ray.tmax) {
        return true;
    } else {
        return false;
    }
}

bool triangle_ray_intersect(const Triangle& triangle, const Ray& ray, Intersect& intersect) {
    Vec3 edge1 = *triangle.positions[1] - *triangle.positions[0];
    Vec3 edge2 = *triangle.positions[2] - *triangle.positions[0];
    Vec3 h = cross(ray.d, edge2);
    float a = dot(edge1, h);
    if (a > -EPSILON && a < EPSILON)
        return false;    // ray parallel to the triangle

    float f = 1.0f/a;
    Vec3 s = ray.o - *triangle.positions[0];
    float u = f * dot(s, h);
    if (u < 0.0f || u > 1.0f)
        return false;
    Vec3 q = cross(s, edge1);
    float v = f * dot(ray.d, q);
    if (v < 0.0f || u + v > 1.0f)
        return false;

    // On calcule t pour savoir ou le point d'intersection se situe sur la ligne.
    float t = f * dot(edge2, q);
    if (t > 0.0f && t < ray.tmax) {
        intersect.t = t;
        intersect.normal =
            (u * (*triangle.normals[1])
             + v * (*triangle.normals[2])
             + (1.0f - u - v) * (*triangle.normals[0])).normalized();
        return true;
    } else {
        return false;
    }
}

void TriangleMesh::calculate_vertex_normals() {
    vertex_normal_.resize(vertex_pos_.size());

    for (const Vec3s& tposi : triangle_pos_indices_) {
        Vec3 edge1 = vertex_pos_[tposi[1]] - vertex_pos_[tposi[0]];
        Vec3 edge2 = vertex_pos_[tposi[2]] - vertex_pos_[tposi[0]];
        Vec3 face_normal = cross(edge1, edge2); // weighted by the triangle area
        for (size_t j = 0; j < 3; j++) {
            vertex_normal_[tposi[j]] += face_normal;
        }
    }

    for (Vec3& normal : vertex_normal_) {
        normal.normalize();
    }

    triangle_normal_indices_ = triangle_pos_indices_;
}

TriangleMesh::TriangleMesh(const std::string& obj_filepath) {
    tinyobj::ObjReader reader;
    tinyobj::ObjReaderConfig config;

    if (!reader.ParseFromFile(obj_filepath, config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader :" << reader.Error();
        }
        throw std::runtime_error("Couldn't load OBJ file");
    }

    if (!reader.Warning().empty()) {
        std::cout << reader.Warning();
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();

    assert(shapes.size() == 1);
    auto& shape = shapes[0];

    size_t index_offset = 0;
    for (size_t f = 0;
         f < shape.mesh.num_face_vertices.size();
         f++) {
        int fv = shape.mesh.num_face_vertices[f];
        assert(fv == 3);

        Vec3s tpos({
            static_cast<size_t>(shape.mesh.indices[index_offset].vertex_index),
            static_cast<size_t>(shape.mesh.indices[index_offset + 1].vertex_index),
            static_cast<size_t>(shape.mesh.indices[index_offset + 2].vertex_index)
        });
        triangle_pos_indices_.push_back(tpos);
        
        Vec3s tnorm({
            static_cast<size_t>(shape.mesh.indices[index_offset].normal_index),
            static_cast<size_t>(shape.mesh.indices[index_offset + 1].normal_index),
            static_cast<size_t>(shape.mesh.indices[index_offset + 2].normal_index)
        });
        triangle_normal_indices_.push_back(tnorm);
        
        index_offset += fv;
    }

    assert(attrib.vertices.size() % 3 == 0);
    vertex_pos_.reserve(attrib.vertices.size() / 3);
    for (size_t v = 0; v < attrib.vertices.size() / 3; v++) {
        vertex_pos_.push_back(Vec3({
                attrib.vertices[v * 3],
                attrib.vertices[v * 3 + 1],
                attrib.vertices[v * 3 + 2]
            }));
    }

    if (attrib.normals.size() == 0) {
        calculate_vertex_normals();
    } else {
        assert(attrib.normals.size() % 3 == 0);
        vertex_normal_.reserve(attrib.normals.size() / 3);
        for (size_t v = 0; v < attrib.normals.size() / 3; v++) {
            vertex_normal_.push_back(Vec3({
                        attrib.normals[v * 3],
                        attrib.normals[v * 3 + 1],
                        attrib.normals[v * 3 + 2]
                    }));
        }
    }

    bvh_ = new BVHNode(BVHNode::from_mesh(*this));
}

TriangleMesh::~TriangleMesh() {
    delete bvh_;
}

TriangleMesh::TriangleMesh(TriangleMesh&& other)
    : vertex_pos_(other.vertex_pos_),
      vertex_normal_(other.vertex_normal_),
      triangle_pos_indices_(other.triangle_pos_indices_),
      triangle_normal_indices_(other.triangle_normal_indices_),
      bvh_(other.bvh_) {
}

bool bvh_intersect(const TriangleMesh& mesh,
                   const BVHNode* node,
                   const Ray& ray) {
    if (!node->box().ray_intersect(ray)) {
        return false;
    }
    
    if (node->is_leaf()) {
        for (size_t idx : node->indices()) {
            if (triangle_ray_intersect(mesh.triangle(idx),
                                       ray)) {
                return true;
            }
        }
        return false;
    } else {
        return bvh_intersect(mesh, node->left(), ray)
            || bvh_intersect(mesh, node->right(), ray);
    }
}

bool bvh_intersect(const TriangleMesh& mesh,
                    const BVHNode* node,
                    const Ray& ray,
                    Intersect& itx) {
    if (!node->box().ray_intersect(ray)) {
        return false;
    }
    // @TODO : make this cleaner (no tmp Intersect object)
    if (node->is_leaf()) {
        bool any_hit = false;
        for (size_t idx : node->indices()) {
            Intersect tri_itx;
            bool hit = triangle_ray_intersect(mesh.triangle(idx),
                                              ray,
                                              tri_itx
                                              );
            if (hit && tri_itx.t < itx.t) {
                itx = tri_itx;
                any_hit = true;
            }
        }
        return any_hit;
    } else {
        Intersect left_itx;
        bool left_hit = bvh_intersect(mesh, node->left(), ray, left_itx);
        if (left_hit && left_itx.t < itx.t) {
            itx = left_itx;
        }
        Intersect right_itx;
        bool right_hit = bvh_intersect(mesh, node->right(), ray, right_itx);
        if (right_hit && right_itx.t < itx.t) {
            itx = right_itx;
        }
        return right_hit || left_hit;
    }
}

bool TriangleMesh::ray_intersect(const Ray& ray) const {
    return bvh_intersect(*this, bvh_, ray);
}

bool TriangleMesh::ray_intersect(const Ray& ray, Intersect& intersect) const {
    return bvh_intersect(*this, bvh_, ray, intersect);
}

Vec3 TriangleMesh::sample(float& pdf) const {
    return Vec3();
}

size_t TriangleMesh::triangle_count() const {
    return triangle_pos_indices_.size();
}

Triangle TriangleMesh::triangle(size_t i) const {
    Triangle t;

    for (size_t j = 0; j < 3; j++) {
        t.positions[j] = &vertex_pos_[triangle_pos_indices_[i][j]];
        t.normals[j] = &vertex_normal_[triangle_normal_indices_[i][j]];
    }

    return t;
}
