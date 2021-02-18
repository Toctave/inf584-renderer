#include "Scene.hpp"

#include <limits>

bool Scene::ray_intersect(Ray& ray, Intersect& itx) const {
    bool result = false;
    for (const Shape* shape : shapes_) {
        if (shape->ray_intersect(ray, itx)) {
            ray.tmax = itx.t;
            result = true;
        }
    }

    return result;
}

void Scene::add_shape(const Shape* shape) {
    shapes_.push_back(shape);
}
