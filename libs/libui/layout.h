#ifndef _LIBUI_LAYOUT_H_
#define _LIBUI_LAYOUT_H_
#include "types.h"

UI_Layout* UI_CreateVBoxLayout();
void UI_SetLayoutContainer(UI_Layout*, UI_Frame*);
void UI_DestroyLayout(UI_Layout*);
void UI_SetLayoutMargin(UI_Layout*,float);
void UI_AddLayoutElement(UI_Layout*, UI_LayoutAlign, float weight);
void UI_ApplyLayout(UI_Layout*);

#endif // _LIBUI_LAYOUT_H_
