#include "../display.hpp"
#include "../util.hpp"
#include "image_analogies.hpp"

#include <fstream>

struct StylitThreadData {
    SyncData& sync;
    ImageAnalogySystem& system;
};

static void display(SyncData& sync, const ImageAnalogySystem& system) {
    SDL_Window* window =
	SDL_CreateWindow("renderer",
			 SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			 640, 480,
			 SDL_WINDOW_RESIZABLE);
    
    bool quit = false;
    size_t lvl = 0;
    
    while (!quit) {
	size_t w = std::max(system.source.unfiltered[lvl].columns(), system.target.unfiltered[lvl].columns());
	size_t h = std::max(system.source.unfiltered[lvl].rows(), system.target.unfiltered[lvl].rows());
	SDL_Surface* surf = SDL_CreateRGBSurface(0,
						 2 * w, 2 * h, 32, 0, 0, 0, 0);
	
	SDL_Rect rect{0, 0, 2 * w, 2 * h};
	SDL_FillRect(surf, &rect, 0);
	
	draw_image(to_rgb8(feature_to_rgb(system.source.unfiltered[lvl])), surf, 0, 0);
	draw_image(to_rgb8(feature_to_rgb(system.source.filtered[lvl])), surf, w, 0);
	draw_image(to_rgb8(feature_to_rgb(system.target.unfiltered[lvl])), surf, 0, h);
	draw_image(to_rgb8(feature_to_rgb(system.target.filtered[lvl])), surf, w, h);
	
	SDL_Surface* winsurf = SDL_GetWindowSurface(window);
	blit_fit(surf, winsurf);
	SDL_FreeSurface(surf);
	
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
		    lvl = (lvl + 1) % system.target.filtered.size();
		    break;
		case SDLK_LEFT:
		    lvl = (lvl + system.target.filtered.size() - 1) % system.target.filtered.size();
		    break;
	    }
	}

	SDL_UpdateWindowSurface(window);
    }
    
    SDL_DestroyWindow(window);
}

int stylit_thread(void* data) {
    StylitThreadData* thread_data = static_cast<StylitThreadData*>(data);
    
    solve(thread_data->system);
    
    return 0;
}

struct StylitArgs {
    FeatureImage images[3];
    std::string output_file;
    size_t levels;
    float kappa;
};

void print_usage() {
    std::cerr << "Usage :";
}

StylitArgs parse_args(int argc, char** argv) {
    StylitArgs args;

    args.levels = 6;
    args.kappa = 0.0f;

    int img_index = 0;
    int i = 1;

    std::vector<Buffer2D<RGBColor>> read_images;
    while (i < argc) {
	std::string arg(argv[i]);
	if (arg == "--kappa") {
	    args.kappa = parse<float>(argv[i+1]);
	    i++;
	} else if (arg == "--levels") {
	    args.levels = parse<size_t>(argv[i+1]);
	    i++;
	} else if (arg == ",") {
	    args.images[img_index] = multichannel_image(read_images);
	    
	    read_images.clear();
	    
	    img_index++;
	    if (img_index > 3) {
		print_usage();
		exit(1);
	    }
	} else {
	    if (img_index < 3) {
		read_images.push_back(to_rgbcolor(read_png(arg)));
	    } else {
		args.output_file = arg;
	    }
	}

	i++;
    }

    return args;
}

int main(int argc, char** argv) {
    SDL_Init(SDL_INIT_VIDEO);
    SyncData sync{ false, SDL_CreateMutex() };

    StylitArgs args = parse_args(argc, argv);
        
    ImageAnalogySystem system(args.images[0],
			      args.images[1],
			      args.images[2],
			      args.levels,
			      args.kappa);

    StylitThreadData thread_data{sync, system};

    SDL_Thread* thread = SDL_CreateThread(stylit_thread, "stylit", &thread_data);
    display(sync, system);

    SDL_WaitThread(thread, nullptr);

    std::ofstream output_file(args.output_file);
    write_png(to_rgb8(feature_to_rgb(system.target.filtered[0])), output_file);
    
    return 0;
}


