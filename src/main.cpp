#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cassert>
#include <sstream>
#include <chrono>
#include <limits>
#include <iomanip>

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
#include "LightTree.hpp"
#include "TOMLParser.hpp"
#include "util.hpp"
#include "display.hpp"

struct Options {
    size_t width;
    size_t height;
    size_t sample_count;
    size_t max_bounces;
    float filter_radius;

    unsigned int seed;

    std::string output_base;

    std::string scene_file;

    std::vector<LightPathExpression> light_paths;
};

void print_usage_string() {
    std::cerr << "Usage : ./renderer [-w width] [-h height] [-s sample_count] scene_file [light paths...]\n";
}

Options parse_options(int argc, char** argv) {
    Options options;
    options.width = 512;
    options.height = 512;
    options.sample_count = std::numeric_limits<size_t>::max();
    options.max_bounces = 3;
    options.filter_radius = 1.5f;
    options.output_base = "out" + timestamp();
    options.seed = time(NULL);

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
        } else if (option == "--seed") {
	    options.seed = parse<unsigned int>(argv[i+1]);
	} else if (option == "-o") {
	    options.output_base = parse<std::string>(argv[i+1]);
	} else {
	    break;
	}
    }

    if (i < argc) {
	options.scene_file = parse<std::string>(argv[i]);
	i++;
    } else {
	print_usage_string();
	exit(1);
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

std::vector<LightTree*> trace_ray(const Scene& scene, Ray& ray, size_t max_bounces = 0) {
    std::vector<LightTree*> results;
    Intersect itx;
    if (scene.ray_intersect(ray, itx)) {
	itx.setup_local_basis();
	
	for (size_t i = 0; i < itx.material->brdfs().size(); i++) {
	    const BRDF* brdf = itx.material->brdfs()[i];
	    results.push_back(new LightTree(brdf->surface_type(), brdf->emit(itx.point, itx.wo)));
	}

	// recursive call :
        if (max_bounces > 0) {
	    for (size_t i = 0; i < results.size(); i++) {
		const BRDF* brdf = itx.material->brdfs()[i];
		
		float pdf;
		Vec3 wi = brdf->sample_wi(itx, itx.wo, &pdf);
		
		if (pdf <= 0.0f) {
		    // skip impossible samples
		    continue;
		}
		
		float cosine_factor = dot(wi, itx.normal);
	    
		Ray bounce(ray.target() + EPSILON * itx.normal,
			   wi);

		RGBColor f = brdf->f(itx,
				     wi,
				     itx.wo);

		Ray bounce_copy(bounce); // to avoid changing bounce.tmax
		std::vector<LightTree*> bounce_trees =
		    trace_ray(scene, bounce_copy, max_bounces - 1);

		for (LightTree* bounce_tree : bounce_trees) {
		    results[i]->add_upstream(bounce_tree,
					     f * cosine_factor / pdf);
		}
	    }
        }

	// direct lighting
	Vec3 hover_point = itx.point + EPSILON * itx.normal;
	for (const Light* light : scene.lights()) {
	    if (light->is_shape(itx.shape)) {
		// don't auto sample
		continue;
	    }
	    
	    LightSample sample = light->sample(hover_point);
	    Vec3 wi = sample.shadow_ray.d.normalized();
	    float cosine_factor = dot(wi, itx.normal);
	    
	    if (!scene.ray_intersect(sample.shadow_ray)) {
		for (size_t i = 0; i < results.size(); i++) {
		    const BRDF* brdf = itx.material->brdfs()[i];
		
		    RGBColor f = brdf->f(itx, wi, itx.wo);
		    
		    LightTree* source_tree = new LightTree(SurfaceType::LIGHT, sample.intensity);
		    results[i]->add_upstream(source_tree,
					     f * cosine_factor / sample.pdf);
		}
	    } 
	}
    } 
    return results;
}

float radians(float deg) {
    return M_PI * deg / 180.0f;
}

double now() {
    static auto t0 = std::chrono::high_resolution_clock::now();

    auto t1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = t1 - t0;
    return diff.count();
}

Vec2 get_image_sample(size_t row, size_t col, size_t width, size_t height, size_t sample_idx) {
    Vec2 random_offset = sample_unit_square();
    return Vec2(static_cast<float>(col), static_cast<float>(row)) + random_offset;
    
    static const size_t grid_size = 4;
    static const float inv_grid_cell_width = 1.0f / grid_size;
    
    size_t i = sample_idx % (grid_size * grid_size);
    size_t gx = i % grid_size;
    size_t gy = i / grid_size;
    

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

void render(SyncData& sync, std::vector<RGBFilm>& output_images, const Options& options, const Scene& scene, const Camera& camera) {
    size_t samples_taken = 0;
    bool need_quit = false;

    double t0 = now();
    double last_time = t0;

    while (samples_taken < options.sample_count && !need_quit) {
#pragma omp parallel for schedule(static, 1)
	for (size_t row = 0; row < options.height; row++) {
	    for (size_t col = 0; col < options.width; col++) {
		Vec2 image_sample = get_image_sample(row, col, options.width, options.height, samples_taken);
		Vec2 screen_sample = to_screen_space(image_sample, options.width, options.height);
		
                Ray camera_ray = camera.get_ray(screen_sample);

		LightTree* eye_tree = new LightTree(SurfaceType::EYE, RGBColor());
		for (LightTree* tree : trace_ray(scene, camera_ray, options.max_bounces)) {
		    eye_tree->add_upstream(tree);
		}

		auto all_radiances = eye_tree->get_all_radiances();
		
		for (size_t i = 0; i < options.light_paths.size(); i++) {
		    RGBColor radiance;
		    for (const auto& p : all_radiances) {
			if (match(options.light_paths[i], p.first)) {
			    radiance += p.second;
			} 
		    }
		    output_images[i].add_sample(image_sample, radiance);
		}

		delete eye_tree;
            }
        }
	samples_taken++;

	double t1 = now();
	double elapsed = t1 - t0;
	double average = elapsed / samples_taken;
	double expected_remaining = average * options.sample_count - elapsed;
	
	std::cout << "sample " << samples_taken << " took "
		  << t1 - last_time << "s ("
		  << formatted_time(elapsed) << " elapsed, "
		  << average << "s avg";
	if (options.sample_count < std::numeric_limits<size_t>::max()) {
	    std::cout << ", " << formatted_time(expected_remaining) << " remaining";
	}
	std::cout << ")\n";
	last_time = t1;

	SDL_LockMutex(sync.mtx);
	if (sync.quit) {
	    need_quit = true;
	}
	SDL_UnlockMutex(sync.mtx);
    }
}

struct RenderThreadData {
    SyncData& sync;
    std::vector<RGBFilm>& output_images;
    const Options& options;
    const Scene& scene;
    const Camera& camera;
};

int render_thread(void* data) {
    RenderThreadData* rtd = static_cast<RenderThreadData*>(data);

    render(rtd->sync, rtd->output_images, rtd->options, rtd->scene, rtd->camera);

    return 0;
}

void update_title(SDL_Window* window, const Options& options, size_t current_img) {
    std::stringstream title;
    title << "renderer - " << options.light_paths[current_img];

    SDL_SetWindowTitle(window, title.str().c_str());
}

void display(SyncData& sync, const std::vector<RGBFilm>& output_images, const Options& options) {
    SDL_Window* window =
	SDL_CreateWindow("renderer",
			 SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			 640, 480,
			 SDL_WINDOW_RESIZABLE);
    
    bool quit = false;
    int current_img = 0;
    SDL_Surface* surf = SDL_CreateRGBSurface(0, options.width, options.height, 32, 0, 0, 0, 0);
    
    update_title(window, options, current_img);
    
    while (!quit) {
	SDL_Surface* winsurf = SDL_GetWindowSurface(window);
	draw_image(output_images[current_img].get_image(), surf, 0, 0);
	
	blit_fit(surf, winsurf);
	
	SDL_Event evt;
	while (SDL_PollEvent(&evt)) {
	    if (evt.type == SDL_QUIT) {
		SDL_LockMutex(sync.mtx);
		sync.quit = true;
		SDL_UnlockMutex(sync.mtx);

		quit = true;
	    }
	    if (evt.type == SDL_KEYDOWN)
		switch (evt.key.keysym.sym) {
		case SDLK_RIGHT:
		    current_img = (current_img + 1) % output_images.size();
		    update_title(window, options, current_img);
		    break;
		case SDLK_LEFT:
		    current_img = (current_img + output_images.size() - 1) % output_images.size();
		    update_title(window, options, current_img);
		    break;
	    }
	}

	SDL_UpdateWindowSurface(window);
    }
    
    SDL_FreeSurface(surf);
    SDL_DestroyWindow(window);
}

int main(int argc, char** argv) {
    Options options = parse_options(argc, argv);
    std::vector<RGBFilm> output_images;
    for (size_t i = 0; i < options.light_paths.size(); i++) {
	output_images.push_back(RGBFilm(options.width, options.height, options.filter_radius));
    }

    TOMLParser parser(options.scene_file, static_cast<float>(options.width) / options.height);
    
    SDL_Init(SDL_INIT_VIDEO);
    initialize_random_system(options.seed);

    SyncData sync{ false, SDL_CreateMutex() };
    
    RenderThreadData data{sync, output_images, options, parser.scene(), parser.camera()};
    SDL_Thread* thread = SDL_CreateThread(render_thread, "render", &data);

    display(sync, output_images, options);

    SDL_WaitThread(thread, nullptr);

    for (size_t i = 0; i < options.light_paths.size(); i++) {
	std::stringstream oss;
	oss << "output/" << options.output_base << "_" << options.light_paths[i] << ".png";

	std::cout << "Writing " << oss.str() << " ...\n";
	
	std::ofstream output_file(oss.str());
	
	Buffer2D<RGBColor> colors = output_images[i].get_colors();
	write_png(to_rgb8(colors), output_file);
    }

    SDL_DestroyMutex(sync.mtx);
    SDL_Quit();
    return 0;
}

