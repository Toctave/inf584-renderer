#include "TriangleMesh.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.hpp"

float triangle_ray_intersect(const Vec3 (&verts)[3], const Ray& ray) {
    Vec3 edge1 = verts[1] - verts[0];
    Vec3 edge2 = verts[2] - verts[0];
    Vec3 h = cross(ray.d, edge2);
    float a = dot(edge1, h);
    if (a > -EPSILON && a < EPSILON)
        return INFTY;    // ray parallel to the triangle

    float f = 1.0f/a;
    Vec3 s = ray.o - verts[0];
    float u = f * dot(s, h);
    if (u < 0.0f || u > 1.0f)
        return INFTY;
    Vec3 q = cross(s, edge1);
    float v = f * dot(ray.d, q);
    if (v < 0.0f || u + v > 1.0f)
        return INFTY;

    // On calcule t pour savoir ou le point d'intersection se situe sur la ligne.
    float t = f * dot(edge2, q);
    if (t > 0.0f && t < ray.tmax) {
        return t;
    } else {
        return INFTY;
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
    for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
        int fv = shape.mesh.num_face_vertices[f];
        assert(fv == 3);

        for (int v = 0; v < fv; v++) {
            tinyobj::index_t idx = shape.mesh.indices[index_offset + v];
            indices_.push_back(idx.vertex_index);
        }

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
}

float TriangleMesh::ray_intersect(const Ray& ray) const {
    float tmin = INFTY;

    for (size_t ti = 0; ti < indices_.size() / 3; ti++) {
        float t = triangle_ray_intersect(
            {vertices_[indices_[ti * 3]], vertices_[indices_[ti * 3 + 1]], vertices_[indices_[ti * 3 + 2]] },
            ray
        );
        if (t < tmin) {
            tmin = t;
        }
    }
    
    return tmin;
}

bool TriangleMesh::ray_intersect(const Ray& ray, Intersect& intersect) const {
    bool any_hit = false;
    for (size_t ti = 0; ti < indices_.size() / 3; ti++) {
        Intersect triangle_itx(ray);
        bool hit = triangle_ray_intersect(
            {vertices_[indices_[ti * 3]], vertices_[indices_[ti * 3 + 1]], vertices_[indices_[ti * 3 + 2]] },
            ray,
            triangle_itx
        );

        if (hit && triangle_itx.t < intersect.t) {
            intersect = triangle_itx;
            any_hit = true;
        }
    }
    
    return any_hit;
}

Vec3 TriangleMesh::sample(float& pdf) const {
    return Vec3();
}
