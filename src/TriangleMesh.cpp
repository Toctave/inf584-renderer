#include "TriangleMesh.hpp"
#include "BVH.hpp"
#include "Sampling.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.hpp"

bool triangle_ray_intersect(const Triangle& triangle, const Ray& ray) {
    Vec3 edge1 = triangle.positions[1] - triangle.positions[0];
    Vec3 edge2 = triangle.positions[2] - triangle.positions[0];
    Vec3 h = cross(ray.d, edge2);
    float a = dot(edge1, h);
    if (a == 0.0f)
        return false;    // ray parallel to the triangle

    float f = 1.0f/a;
    Vec3 s = ray.o - triangle.positions[0];
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
	ray.tmax = t;
        return true;
    } else {
        return false;
    }
}

bool triangle_ray_intersect(const Triangle& triangle, const Ray& ray, Intersect& intersect) {
    Vec3 edge1 = triangle.positions[1] - triangle.positions[0];
    Vec3 edge2 = triangle.positions[2] - triangle.positions[0];
    Vec3 h = cross(ray.d, edge2);
    float a = dot(edge1, h);
    if (a == 0.0f)
        return false;    // ray parallel to the triangle

    float f = 1.0f/a;
    Vec3 s = ray.o - triangle.positions[0];
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
        intersect.normal =
            (u * (triangle.normals[1])
             + v * (triangle.normals[2])
             + (1.0f - u - v) * (triangle.normals[0])).normalized();
	ray.tmax = t;
        return true;
    } else {
        return false;
    }
}

void TriangleMesh::calculate_areas() {
    triangle_areas_.resize(triangle_count());
    triangle_areas_cumsum_.resize(triangle_count());
    total_area_ = 0.0f;

    for (size_t i = 0; i < triangle_count(); i++) {
	Triangle tri = triangle(i);
	Vec3 e1 = tri.positions[1] - tri.positions[0];
	Vec3 e2 = tri.positions[2] - tri.positions[0];

	float area = norm(cross(e1, e2)) / 2.0f;
	triangle_areas_[i] = area;
	total_area_ += area;
	triangle_areas_cumsum_[i] = total_area_;
    }
}

void TriangleMesh::calculate_vertex_normals() {
    // TODO
}

static inline Vec3 get_v3(const std::vector<float>& v, int idx) {
    int ofs = idx * 3;
    return Vec3(v[ofs], v[ofs + 1], v[ofs + 2]);
}

TriangleMesh::TriangleMesh(const std::string& obj_filepath) {
    tinyobj::ObjReader reader;
    tinyobj::ObjReaderConfig config;
    config.triangulate = false;

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

        // triangulate face as a fan
        for (int second = 1; second + 1 < fv; second++) {
            int third = second + 1;
	    tinyobj::index_t id1 = shape.mesh.indices[index_offset];
	    tinyobj::index_t id2 = shape.mesh.indices[index_offset + second];
	    tinyobj::index_t id3 = shape.mesh.indices[index_offset + third];
	    
	    Triangle t;
	    t.positions[0] = get_v3(attrib.vertices, id1.vertex_index);
	    t.positions[1] = get_v3(attrib.vertices, id2.vertex_index);
	    t.positions[2] = get_v3(attrib.vertices, id3.vertex_index);
	    
	    t.normals[0] = get_v3(attrib.normals, id1.normal_index);
	    t.normals[1] = get_v3(attrib.normals, id2.normal_index);
	    t.normals[2] = get_v3(attrib.normals, id3.normal_index);

	    triangles_.push_back(t);
        }
        
        index_offset += fv;
    }

    calculate_areas();
    
    bvh_ = new BVHNode(BVHNode::from_mesh(*this));
}

TriangleMesh::~TriangleMesh() {
    delete bvh_;
}

TriangleMesh::TriangleMesh(TriangleMesh&& other)
    : triangles_(other.triangles_),
      triangle_areas_(other.triangle_areas_),
      triangle_areas_cumsum_(other.triangle_areas_cumsum_),
      total_area_(other.total_area_),
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
		assert(ray.tmax < INFTY);
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
    if (node->is_leaf()) {
        bool any_hit = false;
        for (size_t idx : node->indices()) {
            bool hit = triangle_ray_intersect(mesh.triangle(idx),
                                              ray,
                                              itx
                                              );
	    any_hit = any_hit || hit;
        }
	assert(!any_hit || ray.tmax < INFTY);
        return any_hit;
    } else {
        bool left_hit = bvh_intersect(mesh, node->left(), ray, itx);
        bool right_hit = bvh_intersect(mesh, node->right(), ray, itx);
	assert(!(right_hit || left_hit) || ray.tmax < INFTY);
        return right_hit || left_hit;
    }
}

bool TriangleMesh::ray_intersect(const Ray& ray) const {
    bool result = bvh_intersect(*this, bvh_, ray);
    
    assert(!result || ray.tmax < INFTY);
    
    return result;
}

bool TriangleMesh::ray_intersect(const Ray& ray, Intersect& intersect) const {
    bool result = bvh_intersect(*this, bvh_, ray, intersect);

    assert(!result || ray.tmax < INFTY);

    return result;
}

Vec3 TriangleMesh::sample(float& pdf) const {
    pdf = 1.0f / total_area_;

    float t = random_01() * total_area_;

    // linear search for now
    size_t tri_index = 0;
    while (triangle_areas_cumsum_[tri_index] < t) {
	tri_index++;
    }

    float u = random_01();
    float v = random_01();

    Triangle tri = triangle(tri_index);

    return (1.0f - u - v) * tri.positions[0]
	+ u * tri.positions[1]
	+ v * tri.positions[2];
}

size_t TriangleMesh::triangle_count() const {
    return triangles_.size();
}

const Triangle& TriangleMesh::triangle(size_t i) const {
    return triangles_[i];
}

float TriangleMesh::area() const {
    return total_area_;
}
