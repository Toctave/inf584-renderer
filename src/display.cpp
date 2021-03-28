#include "display.hpp"

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

void draw_image(const Buffer2D<RGB8>& image, SDL_Surface* surface, size_t dx, size_t dy) {
    for (size_t row = 0; row < image.rows(); row++) {
	for (size_t col = 0; col < image.columns(); col++) {
	    RGB8 pixel8 = image(row, col);
	    uint32_t* surface_pixel = ((uint32_t*)surface->pixels) + (row + dy) * surface->w + col + dx;
	    *surface_pixel = SDL_MapRGB(surface->format, pixel8.r, pixel8.g, pixel8.b);
	}
    }
}

