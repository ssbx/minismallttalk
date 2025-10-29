#ifndef _LIBUI_THEMES_H_
#define _LIBUI_THEMES_H_
#include "types.h"

#include <SDL3/SDL.h>

extern UI_Theme* UI_ThemeUsed;
extern UI_Theme* UI_ThemeDefault;

SDL_Color UI_GetThemeColor(UI_ThemeColor);
void UI_GetThemeRGBAFromColor(UI_ThemeColor,Uint8*r,Uint8*g,Uint8*b,Uint8*a);
#endif // _LIBUI_THEMES_H_
