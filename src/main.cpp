#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cassert>
#include <sstream>
#include <chrono>
#include <limits>

#include <SDL2/SDL.h>
#include <ANN/ANN.h>

#include "Vec.hpp"
#include "Matrix.hpp"
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
#include "stylit/stylit.hpp"

struct Options {
    size_t width;
    size_t height;
    size_t sample_count;
    size_t max_bounces;
    float filter_radius;

    std::string output_base;

    std::vector<LightPathExpression> light_paths;
};

template<typename T>
T parse(const std::string& s) {
    T v;
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
    options.sample_count = std::numeric_limits<size_t>::max();
    options.max_bounces = 3;
    options.filter_radius = 1.5f;
    options.output_base = "out";

    int i = 1;
    for (i = 1; i < argc; i += 2) {
        std::string option(argv[i]);
        
        if (option == "-s") {
            options.sample_count = parse<size_t>(argv[i+1]);
        } else if (option == "-w") {
            options.width = parse<size_t>(argv[i+1]);
        } else if (option == "-h") {
            options.height = parse<size_t>(argv[i+1]);
        } else if (option == "-b") {
	    options.max_bounces = parse<size_t>(argv[i+1]);
        } else if (option == "--filter-radius") {
	    options.filter_radius = parse<float>(argv[i+1]);
	} else if (option == "-o") {
	    options.output_base = parse<std::string>(argv[i+1]);
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
	options.light_paths.push_back(LightPathExpression("L*E"));
    }

    return options;
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

bool is_orthonormal_basis(const Vec3& x, const Vec3& y, const Vec3& z) {
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
                local_basis_y = cross(wo + Vec3(1.0f, 0.0f, 0.0f),
                                     itx.normal);
            }
            local_basis_y.normalize();
            Vec3 local_basis_x = cross(itx.normal, local_basis_y);

            assert(is_orthonormal_basis(local_basis_x, local_basis_y, itx.normal));

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
            
            RGBColor f = itx.material->brdf(itx,
                                            wi,
                                            wo);

            float cosine_factor = wi_sample[2];
	    
            LightPath* bounce_path =
                trace_ray(scene, bounce, max_bounces - 1, false);

            if (!bounce_path) {
                return path;
            }

            path->add_tributary(bounce_path,
                                pdf,
                                f,
                                cosine_factor);
        }
        return path;
    }
    return nullptr;
} 

float radians(float deg) {
    return M_PI * deg / 180.0f;
}

void draw_image(const Buffer2D<RGB8>& image, SDL_Surface* surface) {
    for (size_t row = 0; row < image.rows(); row++) {
	for (size_t col = 0; col < image.columns(); col++) {
	    RGB8 pixel8 = image(row, col);
	    uint32_t* surface_pixel = ((uint32_t*)surface->pixels) + row * surface->w + col;
	    *surface_pixel = SDL_MapRGB(surface->format, pixel8.r, pixel8.g, pixel8.b);
	}
    }
}

double now() {
    static auto t0 = std::chrono::high_resolution_clock::now();

    auto t1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = t1 - t0;
    return diff.count();
}

Vec2 get_image_sample(size_t row, size_t col, size_t width, size_t height, size_t sample_idx) {
    static const size_t grid_size = 4;
    static const float inv_grid_cell_width = 1.0f / grid_size;

    size_t i = sample_idx % (grid_size * grid_size);
    size_t gx = i % grid_size;
    size_t gy = i / grid_size;
    
    Vec2 random_offset = sample_unit_square();

    Vec2 jittered_offset(
	(gx + random_offset[0]) * inv_grid_cell_width,
	(gy + random_offset[1]) * inv_grid_cell_width
    );
    
    return Vec2(static_cast<float>(col), static_cast<float>(row)) + jittered_offset;
}

Vec2 to_screen_space(Vec2 sample, size_t width, size_t height) {
    return Vec2(
	2.0f * sample[0] / width - 1.0f,
	1.0f - 2.0f * sample[1] / height
    );
}

std::string formatted_time(double seconds) {
    double minutes = std::floor(seconds / 60.0);
    seconds = std::ceil(seconds - minutes * 60.0f);
    
    double hours = std::floor(minutes / 60.0);
    minutes -= hours * 60.0;

    std::stringstream ss;
    ss << hours << "hrs " << minutes << "m " << seconds << "s";

    return ss.str();
}

void blit_fit(SDL_Surface* src, SDL_Surface* dst) {
    SDL_Rect dst_rect = {0, 0, dst->w, dst->h};
    SDL_FillRect(dst, &dst_rect, SDL_MapRGB(dst->format, 0, 0, 0));
    
    if (src->w * dst->h < dst->w * src->h) {
    	dst_rect.w = dst_rect.h * src->w / src->h;
    	dst_rect.x = (dst->w - dst_rect.w) / 2;
    } else {
    	dst_rect.h = dst_rect.w * src->h / src->w;
    	dst_rect.y = (dst->h - dst_rect.h) / 2;
    }
    
    SDL_BlitScaled(src, NULL, dst, &dst_rect);
}

void render(SDL_Window* window, std::vector<RGBFilm>& output_images, const Options& options) {
    Scene sc;
    SDL_Surface* surf = SDL_CreateRGBSurface(0, options.width, options.height, 32, 0, 0, 0, 0);

    initialize_random_system();

    float fov = radians(45.0f);
    float half_fov = .5f * fov;
    Camera cam(Vec3(0.0f, -1.0f - std::cos(half_fov) / std::sin(half_fov), 1.0f),
               Vec3(0.0f, 0.0f, 1.0f),
               Vec3(0.0f, 0.0f, 1.0f),
               fov,
               static_cast<float>(options.width) / options.height);

    LambertMaterial red(RGBColor(.8f, .3f, .2f));
    LambertMaterial yellow(RGBColor(.9f, .6f, .1f));
    LambertMaterial white(RGBColor::gray(.9f));
    LambertMaterial blue(RGBColor(.3f, .3f, 1.0f));
    
    MicrofacetMaterial glossy(RGBColor(.7f, 1.0f, .7f), 0.5f, .2f, .05f);
    
    Emission emission(20.0f * RGBColor(1.0f, 1.0f, 1.0f));
    
    TriangleMesh teapot_mesh("dragon.obj");
    TriangleMesh box_mesh("box.obj");
    TriangleMesh left_wall_mesh("left_wall.obj");
    TriangleMesh right_wall_mesh("right_wall.obj");

    Sphere light_sphere(Vec3({.0f, 0.0f, 1.8f}), .15f);
    // TriangleMesh light_mesh("light.obj");
    
    Sphere sphere(Vec3({.5f, -.5f, .25f}), .25f);

    Shape teapot(&teapot_mesh, &glossy);
    Shape red_sphere(&sphere, &red);
    Shape box(&box_mesh, &white);
    Shape left_wall(&left_wall_mesh, &yellow);
    Shape right_wall(&right_wall_mesh, &blue);
    Shape light_shape(&light_sphere, &emission);

    teapot.set_transform(Transform::rotate(Vec3(0.0f, 0.0f, 1.0f), radians(135.0f)) * Transform::scale(1.0f));

    sc.add_shape(&box);
    sc.add_shape(&left_wall);
    sc.add_shape(&right_wall);
    
    sc.add_shape(&teapot);
    sc.add_shape(&red_sphere);
    
    sc.add_shape(&light_shape);

    AreaLight light(&light_shape);
    sc.add_light(&light);
    
    size_t samples_taken = 0;
    bool need_quit = false;

    double t0 = now();
    double last_time = t0;

    while (samples_taken < options.sample_count && !need_quit) {
#pragma omp parallel for schedule(static, 4)
	for (size_t row = 0; row < options.height; row++) {
	    for (size_t col = 0; col < options.width; col++) {
		Vec2 image_sample = get_image_sample(row, col, options.width, options.height, samples_taken);
		Vec2 screen_sample = to_screen_space(image_sample, options.width, options.height);
		
                Ray camera_ray = cam.get_ray(screen_sample);

                LightPath* path =
                    trace_ray(sc, camera_ray, options.max_bounces);
                
		for (size_t i = 0; i < options.light_paths.size(); i++) {
		    RGBColor radiance;
		    if (path) {
			radiance = path->radiance_channel(options.light_paths[i]);
		    }
		    output_images[i].add_sample(image_sample, radiance);
		}
		
                if (path) {
                    delete path;
                }
            }
        }
	samples_taken++;

	double t1 = now();
	double elapsed = t1 - t0;
	double average = elapsed / samples_taken;
	double expected_remaining = average * options.sample_count - elapsed;
	
	std::cout << "sample " << samples_taken << " took "
		  << t1 - last_time << "s ("
		  << formatted_time(elapsed) << "s elapsed, "
		  << average << "s avg";
	if (options.sample_count < std::numeric_limits<size_t>::max()) {
	    std::cout << ", " << formatted_time(expected_remaining) << "s remaining";
	}
	std::cout << ")\n";
	last_time = t1;

	draw_image(output_images[0].get_image(), surf);
	blit_fit(surf, SDL_GetWindowSurface(window));
	
	SDL_UpdateWindowSurface(window);
	SDL_Event evt;
	while (SDL_PollEvent(&evt)) {
	    if (evt.type == SDL_QUIT) {
		need_quit = true;
	    }
	}
    }
    SDL_FreeSurface(surf);
}

int main(int argc, char** argv) {
    Options options = parse_options(argc, argv);
    std::vector<RGBFilm> output_images;
    for (size_t i = 0; i < options.light_paths.size(); i++) {
	output_images.push_back(RGBFilm(options.width, options.height, options.filter_radius));
    }
    
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window =
	SDL_CreateWindow("renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_RESIZABLE);

    render(window, output_images, options);

    for (size_t i = 0; i < options.light_paths.size(); i++) {
	std::stringstream oss;
	oss << options.output_base << "_" << options.light_paths[i] << ".png";

	std::cout << "Writing " << oss.str() << " ...\n";
	
	std::ofstream output_file(oss.str());
	
	Buffer2D<RGBColor> colors = output_images[i].get_colors();
	write_png(to_rgb8(colors), output_file);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
