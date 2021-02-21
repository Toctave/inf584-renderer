#include "TriangleMesh.hpp"
#include "BVH.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.hpp"

bool triangle_ray_intersect(const Vec3 (&verts)[3], const Ray& ray) {
    Vec3 edge1 = verts[1] - verts[0];
    Vec3 edge2 = verts[2] - verts[0];
    Vec3 h = cross(ray.d, edge2);
    float a = dot(edge1, h);
    if (a > -EPSILON && a < EPSILON)
        return false;    // ray parallel to the triangle

    float f = 1.0f/a;
    Vec3 s = ray.o - verts[0];
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

bool triangle_ray_intersect(const Vec3 (&verts)[3], const Ray& ray, Intersect& intersect) {
    Vec3 edge1 = verts[1] - verts[0];
    Vec3 edge2 = verts[2] - verts[0];
    Vec3 h = cross(ray.d, edge2);
    float a = dot(edge1, h);
    if (a > -EPSILON && a < EPSILON)
        return false;    // ray parallel to the triangle

    float f = 1.0f/a;
    Vec3 s = ray.o - verts[0];
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
        intersect.normal = cross(edge1, edge2).normalized();
        return true;
    } else {
        return false;
    }
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

        Vec3s triangle({
            static_cast<size_t>(shape.mesh.indices[index_offset].vertex_index),
            static_cast<size_t>(shape.mesh.indices[index_offset + 1].vertex_index),
            static_cast<size_t>(shape.mesh.indices[index_offset + 2].vertex_index)
        });
        
        triangles_.push_back(triangle);
        index_offset += fv;
    }

    assert(attrib.vertices.size() % 3 == 0);
    vertices_.reserve(attrib.vertices.size() / 3);
    for (size_t v = 0; v < attrib.vertices.size() / 3; v++) {
        vertices_.push_back(Vec3({
                attrib.vertices[v * 3],
                attrib.vertices[v * 3 + 1],
                attrib.vertices[v * 3 + 2]
            }));
    }

    bvh_ = new BVHNode(BVHNode::from_mesh(*this));
}

TriangleMesh::~TriangleMesh() {
    delete bvh_;
}

TriangleMesh::TriangleMesh(TriangleMesh&& other)
    : vertices_(other.vertices_),
      triangles_(other.triangles_),
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
            const Vec3s& triangle = mesh.triangles()[idx];

            if (triangle_ray_intersect(
                                       {mesh.vertices()[triangle[0]],
                                        mesh.vertices()[triangle[1]],
                                        mesh.vertices()[triangle[2]]},
                                       ray
                                       )) {
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
            const Vec3s& triangle = mesh.triangles()[idx];
            Intersect tri_itx;
            bool hit = triangle_ray_intersect(
                {mesh.vertices()[triangle[0]],
                 mesh.vertices()[triangle[1]],
                 mesh.vertices()[triangle[2]]},
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

const std::vector<Vec3>& TriangleMesh::vertices() const {
    return vertices_;
}

const std::vector<Vec3s>& TriangleMesh::triangles() const {
    return triangles_;
}
