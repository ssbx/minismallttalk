#ifndef _LIBUI_FRAME_H_
#define _LIBUI_FRAME_H_

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "types.h"
#include "theme.h"
#include "layout.h"
#include "context.h"


void UI_InitBasicFrame(UI_Frame*);
UI_Frame* UI_CreateFrame(UI_FrameType);
UI_Frame* UI_CreateChildFrame(UI_Frame*, UI_FrameType );

void       UI_SetFrameLayout(UI_Frame*, UI_Layout*);
UI_Layout* UI_GetFrameLayout(UI_Frame*);
void       UI_AddSubFrame(UI_Frame*,UI_Frame*);

void UI_ApplyFrameLayout(UI_Frame*);

void UI_DrawFrame(UI_Frame*, SDL_Renderer*);
void UI_DestroyFrame(UI_Frame*);

void UI_SetLabelFrameForegroundColor(UI_Frame*,SDL_Color);
void UI_SetLabelFrameForegroundColorFromTheme(UI_Frame*,UI_ThemeColor);
void UI_SetLabelFrameText(UI_Frame*, const char*, TTF_Font*);
void UI_InitWithContext(UI_Frame*, UI_Context*);

void UI_SetFrameFillColor(UI_Frame*,SDL_Color);
void UI_SetFrameFillColorFromTheme(UI_Frame*,UI_ThemeColor);
void UI_SetFrameFillActive(UI_Frame*,bool);


void UI_SetFrameBroderColor(UI_Frame*,SDL_Color);
void UI_SetFrameBroderColorFromTheme(UI_Frame*,UI_ThemeColor);
void UI_SetFrameBorderActive(UI_Frame*,bool);

void UI_SetFramePosition(UI_Frame*, float,float);
void UI_SetFrameWidth(UI_Frame*, float);
void UI_SetFrameWidthFixed(UI_Frame*,bool);
void UI_SetFrameHeight(UI_Frame*, float);
void UI_SetFrameHeightFixed(UI_Frame*,bool);

void UI_SetFramePadding(UI_Frame*, float top, float bottom, float left, float right);


#endif // _LIBUI_FRAME_H_
