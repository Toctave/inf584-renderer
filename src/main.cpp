#include <iostream>
#include <fstream>

#include "Vec.hpp"
#include "Color.hpp"
#include "Image.hpp"
#include "Ray.hpp"
#include "Sphere.hpp"
#include "Camera.hpp"
#include "Scene.hpp"
#include "Light.hpp"

void fill_bg(RGBImage& output) {
    RGBColor c1({.6, .6, .8});
    RGBColor c2({.8, .8, .5});
    
    for (size_t row = 0; row < output.width(); row++) {
        float ratio = static_cast<float>(row) / output.width();
        RGBColor c = lerp(c1, c2, ratio);
        for (size_t col = 0; col < output.height(); col++) {
            output(col, row) = c;
        }
    }
}

void render(RGBImage& output) {
    fill_bg(output);

    Scene sc;

    Camera cam(Vec3({-2, 0, 0}),
               Vec3({0, 0, 0}),
               Vec3({0, 0, 1}),
               M_PI * .5f,
               static_cast<float>(output.width()) / output.height());

    Sphere s1(Vec3({0, 0, 0}), .5f);
    Sphere s2(Vec3({-.1f, 1, 1}), .2f);
    sc.add_shape(&s2);
    sc.add_shape(&s1);

    PointLight p(Vec3({-1, 1, 1}),
                 RGBColor({1, 1, 1}),
                 2.0f);

    for (size_t row = 0; row < output.height(); row++) {
        Vec2 screen_sample;
        screen_sample[1] =
            1.0f - 2.0f * (static_cast<float>(row) + .5f) / output.height();
        for (size_t col = 0; col < output.width(); col++) {
            screen_sample[0] =
                2.0f * (static_cast<float>(col) + .5f) / output.width() - 1.0f;

            Ray camera_ray = cam.get_ray(screen_sample);
            Intersect itx(camera_ray);


            if (sc.ray_intersect(camera_ray, itx)) {
                Vec3 hit_point = camera_ray.at(itx.t);

                LightSample light_sample = p.sample(hit_point);

                float u = dot(itx.normal, light_sample.wi);
                u = std::max(u, 0.0f);
                
                output(col, row) = u * light_sample.intensity;
                    // RGBColor::from_normal(itx.normal); //(1, 0, 1);
            }
        }
    }
}

int main(int argc, char** argv) {
    RGBImage img(800, 800);
    render(img);
    
    std::ofstream output_file("out.ppm");
    img.output_ppm(output_file);
    return 0;
}
