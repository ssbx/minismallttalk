#include <stdlib.h>
#include "theme.h"

static UI_Theme __baseTheme = {
  { 0x66, 0xD9, 0xEF, 0xFF },
  { 0x27, 0x28, 0x22, 0xFF },
  { 0xF8, 0xF8, 0xF2, 0xFF },
  { 0xFD, 0x97, 0x1F, 0xFF },
  { 0xF8, 0xF8, 0xF0, 0xFF },
  { 0xA6, 0xE2, 0x2E, 0xFF },
  { 0x75, 0x71, 0x5E, 0xFF },
  { 0x38, 0x38, 0x30, 0xFF },
  { 0x59, 0x59, 0x59, 0xFF },
  { 0xE6, 0xE6, 0xE6, 0xFF },
  { 0x49, 0x48, 0x3E, 0xFF },
  { 0xF9, 0x26, 0x72, 0xFF },
  { 0xAE, 0x81, 0xFF, 0xFF },
  { 0xE6, 0xDB, 0x74, 0xFF },
  { 0xFF, 0xFF, 0xFF, 0xFF },
  { 0x00, 0x00, 0x00, 0xFF },
};

UI_Theme* UI_ThemeDefault = &__baseTheme;
UI_Theme* UI_ThemeUsed = &__baseTheme;

SDL_Color UI_GetThemeColor(UI_ThemeColor t) {
  switch (t) {
    case ThemeGlobalColor:
      return UI_ThemeUsed->globalColor;
    case ThemeBackgroundColor:
      return UI_ThemeUsed->backgroundColor;
    case ThemeForegroundColor:
      return UI_ThemeUsed->foregroundColor;
    case ThemeArgumentColor:
      return UI_ThemeUsed->argumentColor;
    case ThemeCaretColor:
      return UI_ThemeUsed->caretColor;
    case ThemeClassColor:
      return UI_ThemeUsed->classColor;
    case ThemeCommentColor:
      return UI_ThemeUsed->commentColor;
    case ThemeGrayDarker:
      return UI_ThemeUsed->grayDarker;
    case ThemeGrayLight:
      return UI_ThemeUsed->grayLight;
    case ThemeGrayLightest:
      return UI_ThemeUsed->grayLightest;
    case ThemeInvisibleColor:
      return UI_ThemeUsed->invisibleColor;
    case ThemeKeywordColor:
      return UI_ThemeUsed->keywordColor;
    case ThemeNumberColor:
      return UI_ThemeUsed->numberColor;
    case ThemeStringColor:
      return UI_ThemeUsed->stringColor;
    case ThemeNamedWhiteColor:
      return UI_ThemeUsed->namedWhiteColor;
    case ThemeNamedBlackColor:
      return UI_ThemeUsed->namedBlackColor;
    default: abort();
  }
}
void UI_GetThemeRGBAFromColor(UI_ThemeColor t, Uint8* r, Uint8* g, Uint8* b, Uint8* a) {
  SDL_Color c = UI_GetThemeColor(t);
  *r = c.r;
  *g = c.g;
  *b = c.b;
  *a = c.a;
}
