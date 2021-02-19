#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cassert>

#include <csignal>
#include <fenv.h>

#include "Vec.hpp"
#include "Color.hpp"
#include "Image.hpp"
#include "Ray.hpp"
#include "Sphere.hpp"
#include "Camera.hpp"
#include "Scene.hpp"
#include "Light.hpp"
#include "Material.hpp"
#include "Sampling.hpp"
#include "TriangleMesh.hpp"
#include "BVH.hpp"

RGBColor background_color(size_t row, size_t col, const RGBImage& img) {
    static const RGBColor c1({.6, .6, .8});
    static const RGBColor c2({.8, .8, .5});
    
    float ratio = static_cast<float>(row) / img.width();
    return lerp(c1, c2, ratio);
}

RGBColor explicit_shade(const Intersect& itx,
                        const Scene& scene,
                        bool keep_lights) {
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

        if (scene.ray_intersect(light_sample.shadow_ray) < 1.0f - EPSILON) {
            continue;
        }

        RGBColor f = itx.material->brdf(itx,
                                        wi,
                                        wo);
        result += u * f * light_sample.intensity / light_sample.pdf;
    }
    if (keep_lights) {
        RGBColor e = itx.material->emit(hit_point, wo);
        result += e;
    }
    
    return result;
}

RGBColor trace_ray(const Scene& scene, Ray& ray, size_t max_bounces = 0, bool keep_lights = true) {
    Intersect itx(ray);
    if (scene.ray_intersect(ray, itx)) {
        if (max_bounces == 0) {
            return explicit_shade(itx, scene, keep_lights);
        } else {
            Vec3 wo = -ray.d;
            Vec3 local_base_y = cross(wo, itx.normal);
            if (local_base_y.norm() < EPSILON) {
                local_base_y = cross(wo + Vec3({1, 0, 0}),
                                     itx.normal);
            }
            local_base_y.normalize();
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
                trace_ray(scene, bounce, max_bounces - 1, false);
            
            RGBColor f = itx.material->brdf(itx,
                                            wi,
                                            wo);
            float u = wi_sample[2]; // cosine factor
            RGBColor c = u * f * incoming_radiance;

            return explicit_shade(itx, scene, keep_lights) + c / pdf;
        }
    } else {
        return RGBColor();
    }
}

void render(RGBImage& output) {
    Scene sc;

    initialize_random_system();

    Camera cam(Vec3({-1.0f, 8.0f, -12.0f}),
               Vec3({0, 0, 0}),
               Vec3({0, 0, 1}),
               M_PI * .5f,
               static_cast<float>(output.width()) / output.height());

    LambertMaterial red(RGBColor(1, 0, 0));
    LambertMaterial white(RGBColor::gray(1.0f));
    Emission blue(300.0f * RGBColor(.5f, .5f, 1.0f));
    
    TriangleMesh mesh("teapot.obj");
    Sphere sphere1(Vec3({0, 0, 0}), 2.0f);
    Sphere sphere2(Vec3({-.5f, 5.0f, .6f}), .2f);
    Sphere sphere3(Vec3({114.0f, .0f, .0f}), 110.0f);
    Sphere sphere4(Vec3({-1.0f, -5.0f, .0f}), .4f);

    Shape shape1(&sphere1, &red);
    Shape shape2(&sphere2, &blue);
    Shape shape3(&sphere3, &white);
    Shape shape4(&sphere4, &blue);
    Shape shape5(&mesh, &red);
    
    /* sc.add_shape(&shape1); */
    sc.add_shape(&shape2);
    sc.add_shape(&shape3);
    sc.add_shape(&shape4);
    sc.add_shape(&shape5);

    PointLight pl1(Vec3({-1, 1, 0}),
                   RGBColor({1, 1, 1}),
                   2.0f);
    PointLight pl2(Vec3({-1, -10, 0}),
                   RGBColor({1, 1, 0}),
                   50.0f);

    AreaLight al1(&shape2);
    AreaLight al2(&shape4);
    sc.add_light(&al1);
    sc.add_light(&al2);
    
    const size_t sample_count = 100;
    const size_t bounces = 3;
    
    for (size_t row = 0; row < output.height(); row++) {
        std::cout << "row " << row << "\n";
        for (size_t col = 0; col < output.width(); col++) {
            for (size_t sample_i = 0; sample_i < sample_count; sample_i++) {
                Vec2 screen_offset =
                    sample_unit_square() * .5f - Vec2({.5f, .5f});
                Vec2 screen_sample({
                        2.0f * (static_cast<float>(col) + screen_offset[0]) / output.width() - 1.0f,
                        1.0f - 2.0f * (static_cast<float>(row) + screen_offset[1]) / output.height()
                    });
            
                Ray camera_ray = cam.get_ray(screen_sample);

                RGBColor radiance = trace_ray(sc, camera_ray, bounces);
                output(col, row) += radiance;
            }

            output(col, row) /= static_cast<float>(sample_count);
        }
    }
}

void fpe_handler(int signum) {
}

int main(int argc, char** argv) {
    /* TriangleMesh mesh("teapot.obj"); */

    /* BVHNode bvh = BVHNode::from_mesh(mesh); */

    /* return 0; */
    
    RGBImage img(256, 256);
    render(img);

    signal(SIGFPE, fpe_handler);
    feenableexcept(FE_INVALID);
    
    std::ofstream output_file("out.ppm");
    img.output_ppm(output_file);
    return 0;
}
