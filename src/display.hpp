#pragma once

#include <SDL2/SDL.h>
#include "Image.hpp"

struct SyncData {
    bool quit;
    SDL_mutex* mtx;
};

void blit_fit(SDL_Surface* src, SDL_Surface* dst);
void draw_image(const Buffer2D<RGB8>& image, SDL_Surface* surface, size_t dx, size_t dy);

