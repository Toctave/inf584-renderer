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

bool Scene::ray_intersect(const Ray& ray) const {
    for (const Shape* shape : shapes_) {
        if (shape->ray_intersect(ray)) {
            return true;
        }
    }

    return false;
}

void Scene::add_shape(const Shape* shape) {
    shapes_.push_back(shape);
}

void Scene::add_light(const Light* light) {
    lights_.push_back(light);
}

const std::vector<const Light*>& Scene::lights() const {
    return lights_;
}

