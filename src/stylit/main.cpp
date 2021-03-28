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
	size_t w = std::max(system.source_unfiltered[lvl].columns(), system.target_unfiltered[lvl].columns());
	size_t h = std::max(system.source_unfiltered[lvl].rows(), system.target_unfiltered[lvl].rows());
	SDL_Surface* surf = SDL_CreateRGBSurface(0,
						 2 * w, 2 * h, 32, 0, 0, 0, 0);
	
	SDL_Rect rect{0, 0, 2 * w, 2 * h};
	SDL_FillRect(surf, &rect, 0);
	
	draw_image(to_rgb8(system.source_unfiltered[lvl]), surf, 0, 0);
	draw_image(to_rgb8(system.source_filtered[lvl]), surf, w, 0);
	draw_image(to_rgb8(system.target_unfiltered[lvl]), surf, 0, h);
	draw_image(to_rgb8(system.target_filtered[lvl]), surf, w, h);
	
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
		    lvl = (lvl + 1) % system.target_filtered.size();
		    break;
		case SDLK_LEFT:
		    lvl = (lvl + system.target_filtered.size() - 1) % system.target_filtered.size();
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

int main(int argc, char** argv) {
    if (argc != 7) {
	return 1;
    }

    SDL_Init(SDL_INIT_VIDEO);
    SyncData sync{ false, SDL_CreateMutex() };
    
    Buffer2D<RGB8> su8 = read_png(argv[1]);
    Buffer2D<RGB8> sf8 = read_png(argv[2]);
    Buffer2D<RGB8> tu8 = read_png(argv[3]);
    
    ImageAnalogySystem system(to_rgbcolor(su8),
			      to_rgbcolor(sf8),
			      to_rgbcolor(tu8),
			      parse<size_t>(argv[5]),
			      parse<float>(argv[6]));

    StylitThreadData thread_data{sync, system};

    SDL_Thread* thread = SDL_CreateThread(stylit_thread, "stylit", &thread_data);
    display(sync, system);

    SDL_WaitThread(thread, nullptr);

    std::ofstream output_file(argv[4]);
    write_png(to_rgb8(system.target_filtered[0]), output_file);
    
    return 0;
}


