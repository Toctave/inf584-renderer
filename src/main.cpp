#include <iostream>
#include <fstream>
#include <cstdlib>

#include "Vec.hpp"
#include "Color.hpp"
#include "Image.hpp"
#include "Ray.hpp"
#include "Sphere.hpp"
#include "Camera.hpp"
#include "Scene.hpp"
#include "Light.hpp"
#include "Material.hpp"

RGBColor background_color(size_t row, size_t col, const RGBImage& img) {
    static const RGBColor c1({.6, .6, .8});
    static const RGBColor c2({.8, .8, .5});
    
    float ratio = static_cast<float>(row) / img.width();
    return lerp(c1, c2, ratio);
}

RGBColor shade(const Intersect& intersect, const Scene& scene) {
    Vec3 hit_point = intersect.incoming.at(intersect.t);
    Vec3 hover_point = hit_point + EPSILON * intersect.normal;
    Vec3 wo = -intersect.incoming.d;

    RGBColor result;
    for (const Light* light : scene.lights()) {
        LightSample light_sample = light->sample(hover_point);
        Vec3 wi = light_sample.shadow_ray.d.normalized();

        float u = dot(intersect.normal, wi);
        if (u < 0.0f) {
            // std::cout << "Behind\n";
            continue;
        }

        if (scene.ray_intersect(light_sample.shadow_ray) < 1.0f) {
            // std::cout << "Shadow\n";
            continue;
        }

        RGBColor f = intersect.material->brdf(intersect,
                                              wi,
                                              wo);
        result += u * f * light_sample.intensity;
    }

    return result;
}

float random_01() {
    return static_cast<float>(rand()) / RAND_MAX;
}

Vec2 random_vec2() {
    return Vec2({random_01(), random_01()});
}

void render(RGBImage& output) {
    Scene sc;

    Camera cam(Vec3({-2, 0, 0}),
               Vec3({0, 0, 0}),
               Vec3({0, 0, 1}),
               M_PI * .5f,
               static_cast<float>(output.width()) / output.height());

    LambertMaterial red(RGBColor(1, 0, 0));
    LambertMaterial blue(RGBColor(.5, .5, 1));
    
    Sphere sphere1(Vec3({0, 0, 0}), .5f);
    Sphere sphere2(Vec3({-.5f, .3f, .3f}), .2f);

    Shape shape1(&sphere1, &red);
    Shape shape2(&sphere2, &blue);
    
    sc.add_shape(&shape2);
    sc.add_shape(&shape1);

    PointLight pl(Vec3({-1, 1, 1}),
                 RGBColor({1, 1, 1}),
                 2.0f);
    sc.add_light(&pl);

    const size_t sample_count = 10;
    for (size_t row = 0; row < output.height(); row++) {
        for (size_t col = 0; col < output.width(); col++) {
            for (size_t sample_i = 0; sample_i < sample_count; sample_i++) {
                Vec2 screen_offset =
                    random_vec2() * .5f - Vec2({.5f, .5f});
                Vec2 screen_sample({
                        2.0f * (static_cast<float>(col) + screen_offset[0]) / output.width() - 1.0f,
                        1.0f - 2.0f * (static_cast<float>(row) + screen_offset[1]) / output.height()
                    });
            
                Ray camera_ray = cam.get_ray(screen_sample);
                Intersect itx(camera_ray);

                if (sc.ray_intersect(camera_ray, itx)) {
                    output(col, row) += shade(itx, sc); //RGBColor(1, 0, 1);
                } else {
                    output(col, row) += background_color(row, col, output);
                }
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
