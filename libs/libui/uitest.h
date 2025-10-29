#ifndef _UITEST__H_
#define _UITEST__H_
#include <SDL3/SDL.h>

extern SDL_Event demoevt;

SDL_Window* createWindow(const char *font);
void handleEvent();
void closeWindow();


#endif // _UITEST__H_
