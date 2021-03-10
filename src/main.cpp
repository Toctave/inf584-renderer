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
#include "stylit/stylit.hpp"

struct Options {
    size_t width;
    size_t height;
    size_t sample_count;
    size_t max_bounces;
    float filter_radius;

    unsigned int seed;

    std::string output_base;

    std::vector<LightPathExpression> light_paths;
};

std::string timestamp()
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream sstream;
    sstream << std::put_time(std::localtime(&now_t), "%Y-%m-%d-%H.%M");
    
    return sstream.str();
}

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
		
		if (pdf == 0.0f) {
		    // skip impossible samples
		    continue;
		}
		
		float cosine_factor = dot(wi, itx.normal);
	    
		Ray bounce(ray.at(itx.t) + EPSILON * itx.normal,
			   wi);

		RGBColor f = itx.material->brdfs()[i]->f(itx,
							 wi,
							 itx.wo);

		Ray bounce_copy(bounce); // to avoid changing bounce.tmax
		std::vector<LightTree*> bounce_trees =
		    trace_ray(scene, bounce_copy, max_bounces - 1);

		for (LightTree* bounce_tree : bounce_trees) {
		    results[i]->add_upstream(bounce_tree,
					     pdf,
					     f,
					     cosine_factor);
		}
	    }
        }
    }
    return results;
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

struct SyncData {
    bool quit;
    SDL_mutex* mtx;
};

void render(SyncData& sync, std::vector<RGBFilm>& output_images, const Options& options) {
    Scene sc;

    initialize_random_system(options.seed);

    float fov = radians(45.0f);
    Camera cam(Vec3(0.0f, -7.0f, 3.0f),
               Vec3(0.0f, 0.0f, 1.0f),
               Vec3(0.0f, 0.0f, 1.0f),
               fov,
               static_cast<float>(options.width) / options.height);

    LambertMaterial red(RGBColor(1.0f, .0f, .0f));
    LambertMaterial yellow(RGBColor(.9f, .6f, .1f));
    LambertMaterial white(RGBColor::gray(.9f));
    LambertMaterial blue(RGBColor(.3f, .3f, 1.0f));
    LambertMaterial green(RGBColor(.7f, 1.0f, .7f));
    
    MicrofacetMaterial glossy(RGBColor(.7f, 1.0f, 1.0f), .05f, .05f);
    MicrofacetMaterial glossy_red(RGBColor(.8f, .0f, .0f), .05f, .05f);
    
    Emission emission(50.0f * RGBColor(1.0f, 1.0f, 1.0f));
    
    TriangleMesh teapot_mesh("dragon.obj");
    TriangleMesh box_mesh("box.obj");
    TriangleMesh left_wall_mesh("left_wall.obj");
    TriangleMesh right_wall_mesh("right_wall.obj");
    TriangleMesh plane_mesh("studio_plane.obj");
    TriangleMesh light_mesh("studio_light.obj");

    Sphere light_sphere(Vec3({.0f, 0.0f, 1.8f}), .15f);
    // TriangleMesh light_mesh("light.obj");
    
    Sphere sphere(Vec3({0.0f, 0.0f, 1.0f}), 1.0f);

    Shape teapot(&teapot_mesh, &glossy_red);
    Shape red_sphere(&sphere, &glossy_red);
    Shape plane(&plane_mesh, &white);
    Shape box(&box_mesh, &white);
    Shape left_wall(&left_wall_mesh, &yellow);
    Shape right_wall(&right_wall_mesh, &blue);
    Shape light_shape(&light_mesh, &emission);

    teapot.set_transform(Transform::translate(.5f, 0.0f, 0.0f) * Transform::rotate(Vec3(0.0f, 0.0f, 1.0f), radians(90.0f)) * Transform::scale(1.0f));

    // sc.add_shape(&box);
    // sc.add_shape(&left_wall);
    // sc.add_shape(&right_wall);

    sc.add_shape(&plane);
    
    sc.add_shape(&teapot);
    // sc.add_shape(&red_sphere);
    
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

		LightTree* eye_tree = new LightTree(SurfaceType::EYE, RGBColor());
		for (LightTree* tree : trace_ray(sc, camera_ray, options.max_bounces)) {
		    eye_tree->add_upstream(tree);
		}

		for (size_t i = 0; i < options.light_paths.size(); i++) {
		    RGBColor radiance = eye_tree->LightTree::radiance_channel(options.light_paths[i]);
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
};

int render_thread(void* data) {
    RenderThreadData* rtd = static_cast<RenderThreadData*>(data);

    render(rtd->sync, rtd->output_images, rtd->options);

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
    size_t current_img = 0;
    SDL_Surface* surf = SDL_CreateRGBSurface(0, options.width, options.height, 32, 0, 0, 0, 0);
    
    update_title(window, options, current_img);
    
    while (!quit) {
	SDL_Surface* winsurf = SDL_GetWindowSurface(window);
	draw_image(output_images[current_img].get_image(), surf);
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
		    current_img = (current_img - 1) % output_images.size();
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
    
    SDL_Init(SDL_INIT_VIDEO);

    SyncData sync{ false, SDL_CreateMutex() };
    
    RenderThreadData data{sync, output_images, options};
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
