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

float Scene::ray_intersect(const Ray& ray) const {
    float tmin = ray.tmax;
    for (const Shape* shape : shapes_) {
        float t = shape->primitive()->ray_intersect(ray);
        if (t < tmin) {
            tmin = t;
        }
    }

    return tmin;
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
