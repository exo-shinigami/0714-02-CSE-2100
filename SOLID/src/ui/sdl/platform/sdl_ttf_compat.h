/**
 * @file sdl_ttf_compat.h
 * @brief SDL_ttf include shim for cross-environment builds and IntelliSense
 */

#ifndef SDL_TTF_COMPAT_H
#define SDL_TTF_COMPAT_H

#if __has_include(<SDL2/SDL_ttf.h>)
#include <SDL2/SDL_ttf.h>
#elif __has_include(<SDL_ttf.h>)
#include <SDL_ttf.h>
#else

#if __has_include(<SDL2/SDL.h>)
#include <SDL2/SDL.h>
#elif __has_include(<SDL.h>)
#include <SDL.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _TTF_Font TTF_Font;

int TTF_Init(void);
const char* TTF_GetError(void);
void TTF_Quit(void);
TTF_Font* TTF_OpenFont(const char* file, int ptsize);
void TTF_CloseFont(TTF_Font* font);
SDL_Surface* TTF_RenderText_Solid(TTF_Font* font, const char* text, SDL_Color fg);
SDL_Surface* TTF_RenderUTF8_Blended(TTF_Font* font, const char* text, SDL_Color fg);

#ifdef __cplusplus
}
#endif

#endif

#endif // SDL_TTF_COMPAT_H
