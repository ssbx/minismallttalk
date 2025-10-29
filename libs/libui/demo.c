#include <stdlib.h>
#include <stdio.h>
#include "theme.h"
#include "window.h"
#include "frame.h"


UI_Window* test2() {
  UI_Window* win = UI_InitWindow(600.,400.);

  UI_Frame* ftop = UI_CreateFrame(BasicFrame);
  UI_SetFrameFillColorFromTheme(ftop, ThemeNamedBlackColor);
  UI_Layout* ltop = UI_CreateVBoxLayout();
  UI_SetLayoutMargin(ltop, 10.);
  UI_SetFrameLayout(ftop, ltop);

  UI_Frame* f2 = UI_CreateChildFrame(ftop, BasicFrame);
  UI_SetFrameFillColorFromTheme(f2, ThemeClassColor);
  UI_SetFrameHeight(f2, 20.);
  UI_SetFrameHeightFixed(f2, true);

  UI_Frame* f3 = UI_CreateChildFrame(ftop, EditboxFrame);

  UI_Frame* f4 = UI_CreateChildFrame(ftop, BasicFrame);
  UI_SetFrameFillColorFromTheme(f4, ThemeCommentColor);
  UI_SetFrameHeight(f4, 20.);
  UI_SetFrameHeightFixed(f4, true);


  UI_SetTopFrameWindow(win, ftop);
  return win;
}


UI_Window* test1() {
  UI_Window* win = UI_InitWindow(600.,400.);

  UI_Frame* ftop = UI_CreateFrame(BasicFrame);
  UI_SetFrameFillColorFromTheme(ftop, ThemeNamedBlackColor);
  UI_Layout* ltop = UI_CreateVBoxLayout();
  UI_SetLayoutMargin(ltop, 10.);
  UI_SetFrameLayout(ftop, ltop);

  UI_Frame* f2 = UI_CreateChildFrame(ftop, BasicFrame);
  UI_SetFrameFillColorFromTheme(f2, ThemeClassColor);
  UI_SetFrameHeight(f2, 20.);
  UI_SetFrameHeightFixed(f2, true);

  UI_Frame* f3 = UI_CreateChildFrame(ftop, BasicFrame);
  UI_SetFrameFillColorFromTheme(f3, ThemeBackgroundColor);

  UI_Frame* txt = UI_CreateChildFrame(ftop, LabelFrame);
  UI_SetLabelFrameForegroundColorFromTheme(txt, ThemeForegroundColor);
  UI_SetLabelFrameText(txt, "Hello", UI_DefaultFont);


  UI_Frame* f4 = UI_CreateChildFrame(ftop, BasicFrame);
  UI_SetFrameFillColorFromTheme(f4, ThemeCommentColor);
  UI_SetFrameHeight(f4, 20.);
  UI_SetFrameHeightFixed(f4, true);


  UI_SetTopFrameWindow(win, ftop);
  return win;
}

int main(int argc, char *argv[]) {

  UI_Init();

  UI_Window *win = test2();

  UI_OpenWindow(win);

  SDL_Event event;
  bool run = true;
  bool need_redraw = true;
  while (run) {

    if (need_redraw) {
      UI_RedrawWindow(win);
      need_redraw = false;
    }
    SDL_WaitEvent(NULL);

    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        case SDL_EVENT_QUIT:
          run = false;
          break;
        case SDL_EVENT_KEY_DOWN:
          { if (event.key.key == SDLK_ESCAPE) run = false; }
          break;
        case SDL_EVENT_WINDOW_RESIZED:
          { UI_NewWindowSize(win); need_redraw = true; }
          break;
        case SDL_EVENT_WINDOW_EXPOSED:
          { need_redraw = true; }
          break;
        default: {}
      }
    }
  }

  UI_DestroyWindow(win);
  UI_Quit();

  return 0;
}
