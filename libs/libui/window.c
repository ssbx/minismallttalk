#include <stdlib.h>
#include "window.h"
#include "theme.h"

UI_Window* UI_InitWindow(float w, float h) {
  UI_Window *win = malloc(sizeof(UI_Window));
  win->top = NULL;
  win->context.window = NULL;
  win->context.renderer = NULL;
  win->clear_color = UI_GetThemeColor(ThemeNamedBlackColor);
  win->width = w;
  win->height = h;
  return win;
}
void UI_SetTopFrameWindow(UI_Window* w, UI_Frame* f) {
  w->top = f;
  UI_SetFrameWidth(f, w->width);
  UI_SetFrameHeight(f, w->height);
  UI_SetFramePosition(f, 0., 0.);
  UI_ApplyFrameLayout(f);
}

void UI_OpenWindow(UI_Window* win) {
  win->context.window = SDL_CreateWindow(
      "libui demo",
      (int) win->width,
      (int) win->height, SDL_WINDOW_RESIZABLE);
  if (!win->context.window) abort();
  win->context.renderer = SDL_CreateRenderer(win->context.window, NULL);
  if (!win->context.renderer) abort();
  SDL_SetRenderVSync(win->context.renderer, 1);
  UI_InitWithContext(win->top, &win->context);
}

void UI_DestroyWindow(UI_Window* win) {
  UI_DestroyFrame(win->top);
  SDL_DestroyRenderer(win->context.renderer);
  SDL_DestroyWindow(win->context.window);
  free(win);
}

void UI_NewWindowSize(UI_Window* win) {
  int w, h;
  SDL_GetWindowSize(win->context.window, &w, &h);
  win->width = (float) w;
  win->height = (float) h;
  UI_SetFrameWidth(win->top, win->width);
  UI_SetFrameHeight(win->top, win->height);
  UI_ApplyFrameLayout(win->top);
}

void UI_RedrawWindow(UI_Window* win) {
  SDL_Color c = win->clear_color;
  SDL_SetRenderDrawColor(win->context.renderer, c.r, c.g, c.b, c.a);
  SDL_RenderClear(win->context.renderer);
  UI_DrawFrame(win->top, win->context.renderer);
  SDL_RenderPresent(win->context.renderer);
}
