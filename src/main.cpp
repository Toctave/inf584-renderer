#include <iostream>
#include <fstream>

#include "Vec.hpp"
#include "Color.hpp"
#include "Image.hpp"
#include "Ray.hpp"
#include "Sphere.hpp"
#include "Camera.hpp"

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
    // fill_bg(output);

    Sphere s(Vec3({0, 0, 0}), .5f);
    Camera cam(Vec3({-2, 0, 0}),
               Vec3({0, 0, 0}),
               Vec3({0, 0, 1}),
               M_PI * .5f,
               static_cast<float>(output.width()) / output.height());

    for (size_t row = 0; row < output.height(); row++) {
        Vec2 sample;
        sample[1] = 1.0f - 2.0f * (static_cast<float>(row) + .5f) / output.height();
        for (size_t col = 0; col < output.width(); col++) {
            sample[0] = 2.0f * (static_cast<float>(col) + .5f) / output.width() - 1.0f;

            Ray ray = cam.get_ray(sample);
            Intersect itx(ray);

            if (s.ray_intersect(ray, itx)) {
                output(col, row) = RGBColor::from_normal(itx.normal); //(1, 0, 1);
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
