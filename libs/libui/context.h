#ifndef _LIBUI_H_
#define _LIBUI_H_

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

extern TTF_Font *UI_BDefaultFont;
extern TTF_Font *UI_BBoldFont;
extern TTF_Font *UI_BObliqueFont;
extern TTF_Font *UI_BBoldObliqueFont;
extern TTF_Font *UI_DefaultFont;
extern TTF_Font *UI_BoldFont;
extern TTF_Font *UI_ObliqueFont;
extern TTF_Font *UI_BoldObliqueFont;

typedef struct UI_Context {
  SDL_Window   *window;
  SDL_Renderer *renderer;
  TTF_Font     *font;
} UI_Context;

void UI_Init();
void UI_Quit();

#endif // _LIBUI_H_

