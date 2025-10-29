#ifndef _LIBUI_TYPES_H_
#define _LIBUI_TYPES_H_

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>
#include <assert.h>
#include "editbox.h"

void printrect(SDL_FRect r);

#define DEFAULT_LAYOUT_MARGIN 2.

typedef enum UI_ExpandFrame {
  ExpandVertical    = 0,
  ExpandHorizontal  = 1,
  ExpandBoth        = 2,
  ExpandNone        = 3,
} UI_ExpandFrame;

typedef enum UI_BoxLayoutOrient {
  OrientVertical = 0,
  OrientHorizontal = 1
} UI_BoxLayoutOrient;

typedef enum UI_LayoutAlign {
  AlignTop      = 0x000000001,
  AlignBottom   = 0x000000010,
  AlignLeft     = 0x000000100,
  AlignRight    = 0x000001000,
  AlignVCenter  = 0x000010000,
  AlignHCenter  = 0x000100000,
  AlignCenter   = 0x000110000
} UI_LayoutAlign;

typedef struct UI_LayoutElem {
  float          weight;
  UI_LayoutAlign align;
} UI_LayoutElem;

typedef union  UI_Frame      UI_Frame;
typedef struct UI_VBoxLayout UI_VBoxLayout;
typedef struct UI_HBoxLayout UI_HBoxLayout;


typedef enum UI_LayoutType {
  VBoxLayout = 0,
  HBoxLayout = 1
} UI_LayoutType;

typedef enum UI_FrameType {
  BasicFrame = 0,
  LabelFrame = 1,
  EditboxFrame = 2
} UI_FrameType;

typedef struct UI_CommonLayout {
  UI_LayoutType      type;
  UI_Frame          *container;
  UI_LayoutElem     *elements;
  int                elements_num;
  int                elements_max;
} UI_CommonLayout;

typedef struct UI_VBoxLayout {
  UI_LayoutType      type;
  UI_Frame          *container;
  UI_LayoutElem     *elements;
  int                elements_num;
  int                elements_max;
  float              margins;
} UI_VBoxLayout;

typedef struct UI_HBoxLayout {
  UI_LayoutType      type;
  UI_Frame          *container;
  UI_LayoutElem     *elements;
  int                elements_num;
  int                elements_max;
  float              margins;
} UI_HBoxLayout;

typedef union UI_Layout {
  UI_LayoutType   type;
  UI_CommonLayout any;
  UI_VBoxLayout   vbox;
  UI_HBoxLayout   hbox;
} UI_Layout;

typedef struct UI_AnyFrame {
  UI_FrameType    type;
  SDL_FRect       rect;
  float           width;
  float           height;
  float           width_min;
  float           width_max;
  bool            width_fixed;
  float           height_min;
  float           height_max;
  bool            height_fixed;
  float           padd_top;
  float           padd_bottom;
  float           padd_left;
  float           padd_right;
  UI_ExpandFrame  expand;
} UI_AnyFrame;

typedef struct UI_EditboxFrame {
  UI_FrameType    type;
  SDL_FRect       rect;
  float           width;
  float           height;
  float           width_min;
  float           width_max;
  bool            width_fixed;
  float           height_min;
  float           height_max;
  bool            height_fixed;
  float           padd_top;
  float           padd_bottom;
  float           padd_left;
  float           padd_right;
  UI_ExpandFrame  expand;
  SDL_Color       fg_color;
  EditBox*        box;
  TTF_Font*       font;
  TTF_TextEngine* engine;
} UI_EditboxFrame;


typedef struct UI_BasicFrame {
  UI_FrameType    type;
  SDL_FRect       rect;
  float           width;
  float           height;
  float           width_min;
  float           width_max;
  bool            width_fixed;
  float           height_min;
  float           height_max;
  bool            height_fixed;
  float           padd_top;
  float           padd_bottom;
  float           padd_left;
  float           padd_right;
  UI_ExpandFrame  expand;
  bool            draw_fill;
  SDL_Color       draw_fill_color;
  bool            draw_border;
  SDL_Color       draw_border_color;
  int             childs_max;
  int             childs_num;
  UI_Frame      **childs;
  UI_Layout      *layout;
} UI_BasicFrame;

typedef struct UI_LabelFrame {
  UI_FrameType    type;
  SDL_FRect       rect;
  float           width;
  float           height;
  float           width_min;
  float           width_max;
  bool            width_fixed;
  float           height_min;
  float           height_max;
  bool            height_fixed;
  float           padd_top;
  float           padd_bottom;
  float           padd_left;
  float           padd_right;
  UI_ExpandFrame  expand;
  SDL_Color       fg_color;
  TTF_Font*       font;
  char*           text;
  SDL_Texture*    texture;
} UI_LabelFrame;

typedef union UI_Frame {
  UI_FrameType   type;
  UI_AnyFrame    any;
  UI_BasicFrame  basic;
  UI_LabelFrame  label;
  UI_EditboxFrame editbox;
} UI_Frame;

typedef enum UI_ThemeColor {
  ThemeGlobalColor = 0,
  ThemeBackgroundColor = 1,
  ThemeForegroundColor = 2,
  ThemeArgumentColor = 3,
  ThemeCaretColor = 4,
  ThemeClassColor = 5,
  ThemeCommentColor = 6,
  ThemeGrayDarker = 7,
  ThemeGrayLight = 8,
  ThemeGrayLightest = 9,
  ThemeInvisibleColor = 10,
  ThemeKeywordColor = 11,
  ThemeNumberColor = 12,
  ThemeStringColor = 13,
  ThemeNamedWhiteColor = 14,
  ThemeNamedBlackColor = 15
} UI_ThemeColor;

#define DEFAULT_FRAME_BORDER_WIDTH 10.
typedef struct UI_Theme {
  SDL_Color globalColor;
  SDL_Color backgroundColor;
  SDL_Color foregroundColor;
  SDL_Color argumentColor;
  SDL_Color caretColor;
  SDL_Color classColor;
  SDL_Color commentColor;
  SDL_Color grayDarker;
  SDL_Color grayLight;
  SDL_Color grayLightest;
  SDL_Color invisibleColor;
  SDL_Color keywordColor;
  SDL_Color numberColor;
  SDL_Color stringColor;
  SDL_Color namedWhiteColor;
  SDL_Color namedBlackColor;
} UI_Theme;


#endif // _LIBUI_TYPES_H_
