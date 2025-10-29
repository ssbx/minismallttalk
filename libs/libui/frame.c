#include "frame.h"
#include "layout.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <SDL3_ttf/SDL_ttf.h>

#define INITIAL_CHILDS_SIZE 4

void printrect(SDL_FRect r) {
  fprintf(stderr, "rect x:%f y:%f w:%f h:%f\n", r.x, r.y, r.w, r.h);
}

void UI_InitAnyFrame(UI_Frame* f) {
  f->any.width = 10.;
  f->any.height = 10.;
  f->any.rect = (SDL_FRect) {0.,0.,f->any.width,f->any.height};
  f->any.width_min = 10.;
  f->any.width_max = -1.;
  f->any.width_fixed = false;
  f->any.height_min = 10.;
  f->any.height_max = -1.;
  f->any.height_fixed = false;
  f->any.padd_top = 0.;
  f->any.padd_bottom = 0.;
  f->any.padd_left = 0.;
  f->any.padd_right = 0.;
  f->any.expand = ExpandBoth;
}

void UI_InitBasicFrame(UI_Frame* f) {
  UI_InitAnyFrame(f);
  f->type = BasicFrame;
  f->basic.draw_fill = true;
  f->basic.draw_fill_color = UI_GetThemeColor(ThemeNamedWhiteColor);
  f->basic.draw_border = false;
  f->basic.draw_border_color = UI_GetThemeColor(ThemeNamedBlackColor);
  f->basic.childs_max = 0;
  f->basic.childs_num = 0;
  f->basic.childs = NULL;
  f->basic.layout = NULL;
}
void UI_InitLabelFrame(UI_Frame* f) {
  UI_InitAnyFrame(f);
  f->type = LabelFrame;
  f->label.text = NULL;
  f->label.texture = NULL;
  f->label.font = NULL;
}

void UI_InitEditboxFrame(UI_Frame* f) {
  UI_InitAnyFrame(f);
  f->type = EditboxFrame;
  f->editbox.box = NULL;
  f->editbox.engine = NULL;
  f->editbox.font = NULL;
  f->editbox.fg_color = UI_GetThemeColor(ThemeForegroundColor);
}


void UI_SetLabelFrameText(UI_Frame*f, const char* txt, TTF_Font* font) {
  if (f->type != LabelFrame) {
    SDL_Log("UI_SetLabelFrameText not possible for type %i\n",f->type);
    abort();
  }
  f->label.text = strdup(txt);
  f->label.font = font;
}

void UI_InitWithContext(UI_Frame*f, UI_Context *c) {
  switch (f->type) {
    case BasicFrame:
      {
        for (int i = 0; i < f->basic.childs_num; i++) {
          UI_InitWithContext(f->basic.childs[i], c);
        }
      }
      break;
    case LabelFrame:
      {
        SDL_Surface *surf = TTF_RenderText_Blended(
            f->label.font,
            f->label.text, 0,
            f->label.fg_color);
        if (!surf) {
          SDL_Log("UI_SetLabelFrameText surface error %s\n", SDL_GetError());
          abort();
        }
        f->label.texture = SDL_CreateTextureFromSurface(c->renderer, surf);
        SDL_DestroySurface(surf);
        if (!f->label.texture) {
          SDL_Log("UI_SetLabelFrameText texture error %s\n", SDL_GetError());
          abort();
        }
      }
      break;
    case EditboxFrame:
      {
        f->editbox.engine = TTF_CreateRendererTextEngine(c->renderer);
        if (!f->editbox.engine) {
          SDL_Log("ttf error %s\n", SDL_GetError());
          abort();
        }
        f->editbox.box = EditBox_Create(c->window, c->renderer,
            f->editbox.engine, f->editbox.font, &f->editbox.rect);
        if (!f->editbox.box) {
          SDL_Log("ttf error %s\n", SDL_GetError());
          abort();
        }
        SDL_Color c = f->editbox.fg_color;
        TTF_SetTextColor(f->editbox.box->text, c.r, c.g, c.b, c.a);
        EditBox_Insert(f->editbox.box, "hello seb");
      }
      break;
  }
}

UI_Frame* UI_CreateFrame(UI_FrameType t) {
  UI_Frame* f = malloc(sizeof(UI_Frame));
  switch (t) {
    case BasicFrame: UI_InitBasicFrame(f); break;
    case LabelFrame: UI_InitLabelFrame(f); break;
    case EditboxFrame: UI_InitEditboxFrame(f); break;
    default: abort();
  }
  return f;
}

UI_Frame* UI_CreateChildFrame(UI_Frame *parent, UI_FrameType t) {
  UI_Frame* f = UI_CreateFrame(t);
  UI_AddSubFrame(parent, f);
  return f;
}

void UI_SetFrameLayout(UI_Frame* f, UI_Layout *l) {
  if (f->type != BasicFrame) {
    SDL_Log("UI_SetFrameLayout fail for frame of type %i\n", f->type);
    abort();
  }
  UI_SetLayoutContainer(l, f);
  f->basic.layout = l;
}

UI_Layout* UI_GetFrameLayout(UI_Frame* f) {
  assert(f->type == BasicFrame);
  return f->basic.layout;
}

void UI_SetFramePosition(UI_Frame* f, float x, float y) {
  f->any.rect.x = x;
  f->any.rect.y = y;
}

void UI_AddSubFrame(UI_Frame* f, UI_Frame *c) {
  if (f->type != BasicFrame) {
    SDL_Log("can not add subframe to type %i\n", f->type);
    abort();
  }
  if (f->basic.childs_max == 0) {
    f->basic.childs = malloc(sizeof(UI_Frame*) * INITIAL_CHILDS_SIZE);
    f->basic.childs_max = INITIAL_CHILDS_SIZE;
    f->basic.childs_num = 0;
  } else if (f->basic.childs_num == f->basic.childs_max) {
    int new_max = f->basic.childs_max * 2;
    f->basic.childs = realloc(f->basic.childs, sizeof(UI_Frame*) * new_max);
    f->basic.childs_max = new_max;
  }
  f->basic.childs[f->basic.childs_num] = c;
  f->basic.childs_num++;
}


int _v = 0;
void UI_ApplyFrameLayout(UI_Frame* f) {
  if (f->type != BasicFrame) {
    SDL_Log("can not apply layout to frame of type %i\n", f->type);
    abort();
  }
  UI_ApplyLayout(f->basic.layout);
  return;
}


void UI_DrawFrame(UI_Frame* f, SDL_Renderer* r) {
  switch (f->type) {
    case BasicFrame:
      {
        SDL_Color c;
        if (f->basic.draw_fill) {
          c = f->basic.draw_fill_color;
          SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
          SDL_RenderFillRect(r, &f->basic.rect);
        }
        if (f->basic.draw_border) {
          c = f->basic.draw_border_color;
          SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
          SDL_RenderFillRect(r, &f->basic.rect);
        }
        for (int i = 0; i < f->basic.childs_num; i++) {
          UI_DrawFrame(f->basic.childs[i], r);
        }
      }
      break;
    case LabelFrame:
      {
        SDL_RenderTexture(r, f->label.texture, NULL, &f->label.rect);
      }
      break;
    case EditboxFrame:
      {
        SDL_Log("prinnnnnn%p\n", f->editbox.box);
        printrect(f->editbox.rect);
        EditBox_SetRect(f->editbox.box, &f->editbox.rect);
        EditBox_Draw(f->editbox.box);
      }
      break;
  }
}

void UI_DestroyFrame(UI_Frame *f) {
  switch (f->type) {
    case BasicFrame:
      {
        if (f->basic.childs) {
          for (int i = 0; i < f->basic.childs_num; i++) {
            UI_DestroyFrame(f->basic.childs[i]);
          }
          free(f->basic.childs);
        }
        if (f->basic.layout) UI_DestroyLayout(f->basic.layout);
      }
      break;
    case LabelFrame:
      {
        if (f->label.text) free(f->label.text);
        if (f->label.texture) SDL_DestroyTexture(f->label.texture);
      }
      break;
    case EditboxFrame:
      {
        if (f->editbox.box) EditBox_Destroy(f->editbox.box);
        if (f->editbox.engine) TTF_DestroyRendererTextEngine(f->editbox.engine);
      }
      break;
  }
  free(f);
}


/***************************************************************************/
void UI_SetLabelFrameForegroundColor(UI_Frame* f, SDL_Color col) {
  assert(f->type == LabelFrame);
  f->label.fg_color = col;}
void UI_SetLabelFrameForegroundColorFromTheme(UI_Frame* f, UI_ThemeColor t) {
  UI_SetLabelFrameForegroundColor(f, UI_GetThemeColor(t));}

/***************************************************************************/
void UI_SetFrameFillColor(UI_Frame* f, SDL_Color col) {
  assert(f->type == BasicFrame);
  f->basic.draw_fill_color = col;}
void UI_SetFrameFillColorFromTheme(UI_Frame* f, UI_ThemeColor t) {
  UI_SetFrameFillColor(f, UI_GetThemeColor(t));}
void UI_SetFrameFillActive(UI_Frame* f, bool val) {
  assert(f->type == BasicFrame);
  f->basic.draw_fill = val;}

/***************************************************************************/
void UI_SetFrameBorderColor(UI_Frame* f, SDL_Color col) {
  assert(f->type == BasicFrame);
  f->basic.draw_border_color = col;}
void UI_SetFrameBorderColorFromTheme(UI_Frame* f, UI_ThemeColor t) {
  UI_SetFrameBorderColor(f, UI_GetThemeColor(t));}
void UI_SetFrameBorderActive(UI_Frame* f, bool val) {
  assert(f->type == BasicFrame);
  f->basic.draw_border = val;}


/***************************************************************************/
void UI_SetFrameWidthFixed(UI_Frame* f, bool v) {
  f->any.width_fixed = v;}
void UI_SetFrameWidth(UI_Frame*f, float w) {
  f->any.rect.w = w; f->any.width = w;}
/***************************************************************************/
void UI_SetFrameHeightFixed(UI_Frame*f, bool v) {
  f->any.height_fixed = v;}
void UI_SetFrameHeight(UI_Frame*f, float h) {
  f->any.rect.h = h; f->any.height = h;}

/***************************************************************************/
void UI_SetFramePadding(UI_Frame* f, float top, float bottom, float left, float right) {
  f->any.padd_top = top;
  f->any.padd_bottom = bottom;
  f->any.padd_left = left;
  f->any.padd_right = right;
}

