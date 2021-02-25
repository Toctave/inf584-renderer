#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cassert>
#include <sstream>

#include <csignal>
#include <fenv.h>

#include <SDL2/SDL.h>

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
#include "LightPath.hpp"

struct Options {
    size_t width;
    size_t height;
    size_t sample_count;

    std::vector<std::string> light_paths;
};

size_t parse_size_t(const std::string& s) {
    size_t v;
    std::stringstream sstream(s);
    sstream >> v;
    return v;
}

void print_usage_string() {
    std::cerr << "Usage : ./renderer -w width -h height -s sample_count [light paths...]\n";
}

Options parse_options(int argc, char** argv) {
    Options options;
    options.width = 64;
    options.height = 64;
    options.sample_count = 10;

    int i = 1;
    for (i = 1; i < argc; i += 2) {
        std::string option(argv[i]);
        
        if (option == "-s") {
            options.sample_count = parse_size_t(argv[i+1]);
        } else if (option == "-w") {
            options.width = parse_size_t(argv[i+1]);
        } else if (option == "-h") {
            options.height = parse_size_t(argv[i+1]);
        } else {
	    break;
	}
    }

    while (i < argc) {
	std::string light_path(argv[i]);
	options.light_paths.push_back(light_path);
	i++;
    }

    if (options.light_paths.size() == 0) {
	options.light_paths.push_back("L*E");
    }

    return options;
}

RGBColor background_color(size_t row, size_t col, const RGBImage& img) {
    static const RGBColor c1({.6, .6, .8});
    static const RGBColor c2({.8, .8, .5});
    
    float ratio = static_cast<float>(row) / img.width();
    return lerp(c1, c2, ratio);
}

void explicit_shade(LightPath* path,
                    Intersect& itx,
                    const Scene& scene,
                    bool keep_lights) {
    Vec3 hover_point = itx.point + EPSILON * itx.normal;

    for (const Light* light : scene.lights()) {
        LightSample light_sample = light->sample(hover_point);
        Vec3 wi = light_sample.shadow_ray.d.normalized();

        float u = dot(itx.normal, wi);
        if (u < 0.0f) {
            continue;
        }

        if (scene.ray_intersect(light_sample.shadow_ray)) {
            continue;
        }

        RGBColor f = itx.material->brdf(itx,
                                        wi,
                                        itx.wo);
        LightPath* emission_path =
            new LightPath(light_sample.intensity,
                          light_sample.shadow_ray.at(1.0f));
        path->add_tributary(emission_path,
                            light_sample.pdf,
                            f,
                            u);
    }
    if (keep_lights) {
        RGBColor e = itx.material->emit(itx.point, itx.wo);
        path->set_emission(e);
    }
}

bool orthonormal_basis(const Vec3& x, const Vec3& y, const Vec3& z) {
    return (fabs(x.norm() - 1.0f) < EPSILON)
        && (fabs(y.norm() - 1.0f) < EPSILON)
        && (fabs(z.norm() - 1.0f) < EPSILON)
        && (fabs(dot(x, y)) < EPSILON)
        && (fabs(dot(y, z)) < EPSILON)
        && (fabs(dot(x, z)) < EPSILON);
}

LightPath* trace_ray(const Scene& scene,
                     Ray& ray,
                     size_t max_bounces = 0,
                     bool keep_lights = true) {
    Intersect itx;
    if (scene.ray_intersect(ray, itx)) {
        LightPath* path = new LightPath(itx.material->surface_type(),
                                        itx.point);
        explicit_shade(path, itx, scene, keep_lights);
        
        if (max_bounces > 0) {
            Vec3 wo = -ray.d;
            Vec3 local_basis_y = cross(wo, itx.normal);
            if (local_basis_y.norm() < EPSILON) {
                local_basis_y = cross(wo + Vec3({1, 0, 0}),
                                     itx.normal);
            }
            local_basis_y.normalize();
            Vec3 local_basis_x = cross(itx.normal, local_basis_y);

            assert(orthonormal_basis(local_basis_x, local_basis_y, itx.normal));

            float pdf;
            Vec3 wi_sample = sample_hemisphere_cosine_weighted(&pdf);

            if (pdf == 0.0f) {
                // skip impossible samples
                return path;
            }
            
            Vec3 wi = wi_sample[0] * local_basis_x
                + wi_sample[1] * local_basis_y
                + wi_sample[2] * itx.normal;

            Ray bounce(ray.at(itx.t) + EPSILON * itx.normal,
                       wi);
            
            LightPath* bounce_path =
                trace_ray(scene, bounce, max_bounces - 1, false);

            if (!bounce_path) {
                return path;
            }
            
            RGBColor f = itx.material->brdf(itx,
                                            wi,
                                            wo);
            float u = wi_sample[2]; // cosine factor

            path->add_tributary(bounce_path,
                                pdf,
                                f,
                                u);
        }
        return path;
    }
    return nullptr;
} 

float radians(float deg) {
    return M_PI * deg / 180.0f;
}

void draw_image(const RGBImage& image, SDL_Surface* surface, size_t samples) {
    float scale = 1.0f / samples;
    for (size_t row = 0; row < image.height(); row++) {
	for (size_t col = 0; col < image.width(); col++) {
	    RGBColor color = scale * image(col, row);
	    RGB8 pixel8 = color.to_8bit();
	    uint32_t* surface_pixel = ((uint32_t*)surface->pixels) + row * image.width() + col;
	    *surface_pixel = SDL_MapRGB(surface->format, pixel8.r, pixel8.g, pixel8.b);
	}
    }
}

void render(SDL_Window* window, std::vector<RGBImage>& output_images, const Options& options) {
    Scene sc;

    initialize_random_system();

    float fov = radians(45.0f);
    float half_fov = .5f * fov;
    Camera cam(Vec3({0.0f, -1.0f - std::cos(half_fov) / std::sin(half_fov), 1.0f}),
               Vec3({0, 0, 1}),
               Vec3({0, 0, 1}),
               fov,
               static_cast<float>(options.width) / options.height);

    LambertMaterial red(RGBColor(.5f, 0, 0));
    LambertMaterial yellow(RGBColor(.9f, .6f, .1f));
    LambertMaterial white(RGBColor::gray(.5f));
    LambertMaterial blue(RGBColor(.3f, .3f, 1.0f));
    
    MicrofacetMaterial glossy(.2f, 1.0f);
    
    Emission emission(50.0f * RGBColor(1.0f, 1.0f, 1.0f));
    
    TriangleMesh teapot_mesh("teapot.obj");
    TriangleMesh box_mesh("box.obj");
    TriangleMesh left_wall_mesh("left_wall.obj");
    TriangleMesh right_wall_mesh("right_wall.obj");

    Sphere light_sphere(Vec3({.0f, .0f, 1.8f}), .15f);
    
    Sphere sphere(Vec3({.5f, .5f, .3f}), .3f);

    Shape teapot(&teapot_mesh, &glossy);
    Shape red_sphere(&sphere, &red);
    Shape box(&box_mesh, &white);
    Shape left_wall(&left_wall_mesh, &yellow);
    Shape right_wall(&right_wall_mesh, &blue);
    Shape light_shape(&light_sphere, &emission);
    
    sc.add_shape(&box);
    sc.add_shape(&left_wall);
    sc.add_shape(&right_wall);
    
    sc.add_shape(&teapot);
    sc.add_shape(&red_sphere);
    
    sc.add_shape(&light_shape);

    AreaLight light(&light_shape);
    sc.add_light(&light);
    
    const size_t bounces = 10;
    size_t samples_taken = 0;
    bool need_quit = false;

    while (samples_taken < options.sample_count && !need_quit) {
	std::cout << "sample " << samples_taken + 1 << "\n";
#pragma omp parallel for schedule(static, 4)
	for (size_t row = 0; row < options.height; row++) {
	    for (size_t col = 0; col < options.width; col++) {
                Vec2 screen_offset =
                    sample_unit_square() * .5f - Vec2({.5f, .5f});
                Vec2 screen_sample({
                        2.0f * (static_cast<float>(col) + screen_offset[0]) / options.width - 1.0f,
                        1.0f - 2.0f * (static_cast<float>(row) + screen_offset[1]) / options.height
                    });
            
                Ray camera_ray = cam.get_ray(screen_sample);

                LightPath* path =
                    trace_ray(sc, camera_ray, bounces);
                
                if (path) {
		    for (size_t i = 0; i < options.light_paths.size(); i++) {
			RGBColor radiance = path->radiance_channel(options.light_paths[i]);
			
			output_images[i](col, row) += radiance;
		    }
                
                    delete path;
                }
            }

        }
	samples_taken++;
	
	draw_image(output_images[0], SDL_GetWindowSurface(window), samples_taken + 1);
	
	SDL_UpdateWindowSurface(window);
	SDL_Event evt;
	while (SDL_PollEvent(&evt)) {
	    if (evt.type == SDL_QUIT) {
		need_quit = true;
	    }
	}
    }
    
    for (size_t i = 0; i < options.light_paths.size(); i++) {
	for (size_t row = 0; row < options.height; row++) {
	    for (size_t col = 0; col < options.width; col++) {
		output_images[i](col, row) /= static_cast<float>(samples_taken);
	    }
	}
    }
}

void fpe_handler(int signum) {
}

int main(int argc, char** argv) {
    Options options = parse_options(argc, argv);
    std::vector<RGBImage> output_images;
    for (size_t i = 0; i < options.light_paths.size(); i++) {
	output_images.push_back(RGBImage(options.width, options.height));
    }
    
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window =
	SDL_CreateWindow("renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, options.width, options.height, 0);

    render(window, output_images, options);

    signal(SIGFPE, fpe_handler);
    feenableexcept(FE_INVALID);

    for (size_t i = 0; i < options.light_paths.size(); i++) {
	std::ofstream output_file("out" + options.light_paths[i] + ".png");
	output_images[i].output_png(output_file);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
