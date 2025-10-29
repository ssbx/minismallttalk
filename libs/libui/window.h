#ifndef _LIBUI_WINDOW_H_
#define _LIBUI_WINDOW_H_

#include "frame.h"
#include "context.h"

typedef struct UI_Window {
  UI_Frame*  top;
  UI_Context context;
  SDL_Color  clear_color;
  float width;
  float height;
} UI_Window;

UI_Window* UI_InitWindow(float w, float h);
void       UI_OpenWindow(UI_Window*);
void       UI_RedrawWindow(UI_Window*);
void       UI_DestroyWindow(UI_Window*);
void       UI_SetTopFrameWindow(UI_Window*, UI_Frame*);
void       UI_NewWindowSize(UI_Window*);

#endif // _LIBUI_WINDOW_H_
