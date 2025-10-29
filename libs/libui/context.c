#include "context.h"
#include <stdlib.h>

static bool __sdl_initialized = false;
static bool __ttf_initialized = false;

TTF_Font *UI_BDefaultFont = NULL;
TTF_Font *UI_BBoldFont = NULL;
TTF_Font *UI_BObliqueFont = NULL;
TTF_Font *UI_BBoldObliqueFont = NULL;

TTF_Font *UI_DefaultFont = NULL;
TTF_Font *UI_BoldFont = NULL;
TTF_Font *UI_ObliqueFont = NULL;
TTF_Font *UI_BoldObliqueFont = NULL;


void UI_Init(const char *font) {
  if (!__sdl_initialized) {
    if (!SDL_Init(SDL_INIT_VIDEO & SDL_INIT_EVENTS)) {
      SDL_Log("failed to init sdl %s\n", SDL_GetError());
      abort();
    }
    __sdl_initialized = true;
  }
  if (!__ttf_initialized) {
    if (!TTF_Init()) {
      SDL_Log("failed to init ttf %s\n", SDL_GetError());
      abort();
    }
    UI_DefaultFont = TTF_OpenFont("fonts/DejaVuSans.ttf", 11.5);
    if (!UI_DefaultFont) SDL_Log("faile open font%s\n", SDL_GetError());
    UI_BoldFont = TTF_OpenFont("fonts/DejaVuSans-Bold.ttf", 11.5);
    if (!UI_BoldFont ) SDL_Log("faile open font%s\n", SDL_GetError());
    UI_ObliqueFont = TTF_OpenFont("fonts/DejaVuSans-Oblique.ttf", 11.5);
    if (!UI_ObliqueFont ) SDL_Log("faile open font%s\n", SDL_GetError());
    UI_BoldObliqueFont = TTF_OpenFont("fonts/DejaVuSans-BoldOblique.ttf", 11.5);
    if (!UI_BoldObliqueFont ) SDL_Log("faile open font%s\n", SDL_GetError());

    UI_BDefaultFont = TTF_OpenFont("fonts/DejaVuSans.ttf", 14.5);
    if (!UI_BDefaultFont ) SDL_Log("faile open font%s\n", SDL_GetError());
    UI_BBoldFont = TTF_OpenFont("fonts/DejaVuSans-Bold.ttf", 14.5);
    if (!UI_BBoldFont ) SDL_Log("faile open font%s\n", SDL_GetError());
    UI_BObliqueFont = TTF_OpenFont("fonts/DejaVuSans-Oblique.ttf", 14.5);
    if (!UI_BObliqueFont ) SDL_Log("faile open font%s\n", SDL_GetError());
    UI_BBoldObliqueFont = TTF_OpenFont("fonts/DejaVuSans-BoldOblique.ttf", 14.5);
    if (!UI_BBoldObliqueFont ) SDL_Log("faile open font%s\n", SDL_GetError());

    __ttf_initialized = true;
  }
}

void UI_Quit() {

  if (UI_BDefaultFont) TTF_CloseFont(UI_BDefaultFont);
  if (UI_BBoldFont) TTF_CloseFont(UI_BBoldFont);
  if (UI_BObliqueFont) TTF_CloseFont(UI_BObliqueFont);
  if (UI_DefaultFont) TTF_CloseFont(UI_DefaultFont);
  if (UI_BoldFont) TTF_CloseFont(UI_BoldFont);
  if (UI_ObliqueFont) TTF_CloseFont(UI_ObliqueFont);
  if (UI_BoldObliqueFont) TTF_CloseFont(UI_BoldObliqueFont);

  TTF_Quit();
  SDL_Quit();
}
