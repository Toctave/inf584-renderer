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
#include "Material.hpp"

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

void render(RGBImage& output) {
    fill_bg(output);

    Scene sc;

    Camera cam(Vec3({-2, 0, 0}),
               Vec3({0, 0, 0}),
               Vec3({0, 0, 1}),
               M_PI * .5f,
               static_cast<float>(output.width()) / output.height());

    LambertMaterial red(RGBColor(1, 0, 0));
    
    Sphere sphere1(Vec3({0, 0, 0}), .5f);
    Sphere sphere2(Vec3({-.5f, .3f, .3f}), .2f);

    Shape shape1(&sphere1, &red);
    Shape shape2(&sphere2, &red);
    
    sc.add_shape(&shape2);
    sc.add_shape(&shape1);

    PointLight pl(Vec3({-1, 1, 1}),
                 RGBColor({1, 1, 1}),
                 2.0f);
    sc.add_light(&pl);

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
                output(col, row) = shade(itx, sc); //RGBColor(1, 0, 1);
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
