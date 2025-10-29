#include "layout.h"
#include <stdlib.h>
#include <stdio.h>

UI_Layout* UI_CreateVBoxLayout() {
  UI_Layout* l = malloc(sizeof(UI_Layout));
  l->type = VBoxLayout;
  l->vbox.container = NULL;
  l->vbox.margins = .5;
  l->vbox.elements = NULL;
  l->vbox.elements_num = 0;
  l->vbox.elements_max = 0;
  return l;
}

void UI_SetLayoutContainer(UI_Layout* l, UI_Frame *f) {
  l->any.container = f;
}

void UI_DestroyLayout(UI_Layout* l) {
  if (l->any.elements) free(l->any.elements);
  free(l);
}

void UI_SetLayoutMargin(UI_Layout* l, float v) {
  switch (l->type) {
    case VBoxLayout:
    case HBoxLayout:
      l->hbox.margins = v;
      break;
  }
}

void __applyVBoxLayout(UI_VBoxLayout* l) {

  if (l->container->type != BasicFrame) return;
  UI_BasicFrame *ct  = &l->container->basic;
  SDL_FRect r   = ct->rect;

  float fix_size = 0.0f;
  int   fix_num = 0;
  for (int i = 0; i < ct->childs_num; i++) {
    UI_AnyFrame *ch = &ct->childs[i]->any;
    if (ch->height_fixed) {
      fix_num++;
      fix_size += ch->height;
    }
  };

  float space_remains = r.h - fix_size;
  float ch_height = space_remains / (ct->childs_num - fix_num);

  float y_pos = r.y;
  for (int i = 0; i < ct->childs_num; i++) {
    UI_AnyFrame *ch = &ct->childs[i]->any;
    if (ch->height_fixed) {
      ch->rect.x = r.x;
      ch->rect.y = y_pos;
      ch->rect.w = r.w;
      ch->rect.h = ch->height;
      y_pos += ch->height;
    } else {
      ch->rect.x = r.x;
      ch->rect.y = y_pos;
      ch->rect.w = r.w;
      ch->rect.h = ch_height;
      y_pos += ch_height;
    }
  }
}

void __applyHBoxLayout(UI_HBoxLayout* l) {
}

void UI_ApplyLayout(UI_Layout* l) {

  if (l == NULL) return;

  switch (l->type) {
    case VBoxLayout:
      __applyVBoxLayout(&l->vbox);
    case HBoxLayout:
      __applyHBoxLayout(&l->hbox);
    default: {}
  }

  UI_Frame *ct = l->any.container;
  assert(ct->type == BasicFrame);
  for (int i = 0; i < ct->basic.childs_num; i++) {
    UI_Frame *child = ct->basic.childs[i];
    if (child->type == BasicFrame)
      UI_ApplyLayout(child->basic.layout);
  }
}
