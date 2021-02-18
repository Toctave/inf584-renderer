#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cassert>

#include "Vec.hpp"
#include "Color.hpp"
#include "Image.hpp"
#include "Ray.hpp"
#include "Sphere.hpp"
#include "Camera.hpp"
#include "Scene.hpp"
#include "Light.hpp"
#include "Material.hpp"

float random_01() {
    return static_cast<float>(rand()) / RAND_MAX;
}

Vec2 sample_unit_square() {
    return Vec2({random_01(), random_01()});
}

Vec2 sample_unit_disc() {
    Vec2 sq = sample_unit_square();
    float theta = 2.0f * M_PI * sq[0];
    float r = std::sqrt(sq[1]);
    
    return Vec2({r * std::cos(theta), r * std::sin(theta)});
}

Vec3 sample_hemisphere_cosine_weighted(float* pdf) {
    Vec2 disc = sample_unit_disc();

    Vec3 sample({disc[0], disc[1], std::sqrt(1.0f - disc.norm_squared())});

    *pdf = sample[2] * disc.norm() / M_PI;

    return sample;
}

RGBColor background_color(size_t row, size_t col, const RGBImage& img) {
    static const RGBColor c1({.6, .6, .8});
    static const RGBColor c2({.8, .8, .5});
    
    float ratio = static_cast<float>(row) / img.width();
    return lerp(c1, c2, ratio);
}

RGBColor explicit_shade(const Intersect& itx, const Scene& scene) {
    Vec3 hit_point = itx.incoming.at(itx.t);
    Vec3 hover_point = hit_point + EPSILON * itx.normal;
    Vec3 wo = -itx.incoming.d;

    RGBColor result;
    for (const Light* light : scene.lights()) {
        LightSample light_sample = light->sample(hover_point);
        Vec3 wi = light_sample.shadow_ray.d.normalized();

        float u = dot(itx.normal, wi);
        if (u < 0.0f) {
            continue;
        }

        if (scene.ray_intersect(light_sample.shadow_ray) < 1.0f) {
            continue;
        }

        RGBColor f = itx.material->brdf(itx,
                                        wi,
                                        wo);
        result += u * f * light_sample.intensity;
    }
    RGBColor e = itx.material->emit(itx, wo);
    result += e;

    return result;
}

RGBColor trace_ray(const Scene& scene, Ray& ray, size_t max_bounces = 0) {
    Intersect itx(ray);
    if (scene.ray_intersect(ray, itx)) {
        if (max_bounces == 0) {
            return explicit_shade(itx, scene);
        } else {
            Vec3 wo = -ray.d;
            Vec3 local_base_y = cross(wo, itx.normal).normalized();
            Vec3 local_base_x = cross(itx.normal, local_base_y);

            assert(fabs(local_base_x.norm() - 1.0f) < EPSILON);
            assert(fabs(local_base_y.norm() - 1.0f) < EPSILON);
            assert(fabs(dot(local_base_x, local_base_y)) < EPSILON);
            assert(fabs(dot(local_base_x, itx.normal)) < EPSILON);
            assert(fabs(dot(local_base_y, itx.normal)) < EPSILON);

            float pdf;
            Vec3 wi_sample = sample_hemisphere_cosine_weighted(&pdf);
            Vec3 wi = wi_sample[0] * local_base_x
                + wi_sample[1] * local_base_y
                + wi_sample[2] * itx.normal;
            
            Ray bounce(ray.at(itx.t) + EPSILON * itx.normal,
                       wi);
            RGBColor incoming_radiance =
                trace_ray(scene, bounce, max_bounces - 1);
            
            RGBColor f = itx.material->brdf(itx,
                                            wi,
                                            wo);
            float u = wi_sample[2]; // cosine factor
            RGBColor c = u * f * incoming_radiance;

            return explicit_shade(itx, scene) + c / pdf;
        }
    } else {
        return RGBColor();
    }
}

void render(RGBImage& output) {
    Scene sc;

    Camera cam(Vec3({-2, 0, 0}),
               Vec3({0, 0, 0}),
               Vec3({0, 0, 1}),
               M_PI * .5f,
               static_cast<float>(output.width()) / output.height());

    LambertMaterial red(RGBColor(1, 0, 0));
    LambertMaterial white(RGBColor::gray(1.0f));
    Emission blue(100.0f * RGBColor(.5f, .5f, 1.0f));
    
    Sphere sphere1(Vec3({0, 0, 0}), .5f);
    Sphere sphere2(Vec3({-.5f, .3f, .3f}), .2f);
    Sphere sphere3(Vec3({12.0f, .0f, .0f}), 10.0f);

    Shape shape1(&sphere1, &red);
    Shape shape2(&sphere2, &blue);
    Shape shape3(&sphere3, &white);
    
    sc.add_shape(&shape1);
    sc.add_shape(&shape2);
    sc.add_shape(&shape3);

    PointLight pl1(Vec3({-1, 1, 0}),
                 RGBColor({1, 1, 1}),
                 2.0f);
    PointLight pl2(Vec3({-1, -10, 0}),
                 RGBColor({1, 1, 0}),
                 50.0f);

    sc.add_light(&pl1);
    
    const size_t sample_count = 1000;
    const size_t bounces = 3;
    for (size_t row = 0; row < output.height(); row++) {
        for (size_t col = 0; col < output.width(); col++) {
            for (size_t sample_i = 0; sample_i < sample_count; sample_i++) {
                Vec2 screen_offset =
                    sample_unit_square() * .5f - Vec2({.5f, .5f});
                Vec2 screen_sample({
                        2.0f * (static_cast<float>(col) + screen_offset[0]) / output.width() - 1.0f,
                        1.0f - 2.0f * (static_cast<float>(row) + screen_offset[1]) / output.height()
                    });
            
                Ray camera_ray = cam.get_ray(screen_sample);

                output(col, row) += trace_ray(sc, camera_ray, bounces);
            }

            output(col, row) /= static_cast<float>(sample_count);
        }
    }
}

int main(int argc, char** argv) {
    RGBImage img(200, 200);
    render(img);
    
    std::ofstream output_file("out.ppm");
    img.output_ppm(output_file);
    return 0;
}
