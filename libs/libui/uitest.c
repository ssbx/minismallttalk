#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <stdio.h>
#include <stdlib.h>

#include "libui/editbox.h"

#define DEFAULT_PTSIZE  18.0f
#define DEFAULT_TEXT    "The quick brown fox jumped over the lazy dog"
// Newlines
//#define DEFAULT_TEXT    "The quick brown fox\njumped over the lazy dog"
// Combining characters
//#define DEFAULT_TEXT    "\xc5\xab\xcc\x80\x20\xe1\xba\x83\x20\x6e\xcc\x82\x20\x48\xcc\xa8\x20\x6f\xcd\x9c\x75"
// Chinese text
//#define DEFAULT_TEXT    "\xe5\xad\xa6\xe4\xb9\xa0\xe6\x9f\x90\xe8\xaf\xbe\xe7\xa8\x8b\xe5\xbf\x85\xe8\xaf\xbb\xe7\x9a\x84"
// Hebrew text
//#define DEFAULT_TEXT      "\xd7\xa6\xd7\xa4\xd7\xa2\x20\xd7\x97\xd7\x96\xd7\xa7\x20\xd7\xa0\xd7\xa9\xd7\x9a\x20\xd7\x93\xd7\x92\x20\xd7\x9e\xd7\xaa\x20\xd7\x91\xd7\x90\xd7\x95\xd7\xa1\xd7\x98\xd7\xa8\xd7\x9c\xd7\x99\xd7\x94"
// Mixed English, Chinese, Hebrew, and emoji text
//#define DEFAULT_TEXT    "The quick brown fox\njumped over the \xe5\xad\xa6\xe4\xb9\xa0\xe6\x9f\x90\xe8\xaf\xbe\xe7\xa8\x8b\xe5\xbf\x85\xe8\xaf\xbb\xe7\x9a\x84 \xd7\x90\xd7\x91\xd7\x92 \xf0\x9f\x98\x89"
#define WIDTH   640
#define HEIGHT  480
#define MAX_FALLBACKS 4


#define TTF_SHOWFONT_USAGE \
  "Usage: %s [--textengine surface|renderer] [--solid] [--shaded] [--blended] [-b] [-i] [-u] [-s] [--outline size] [--hintlight|--hintmono|--hintnone] [--nokerning] [--wrap] [--align left|center|right] [--fgcol r,g,b,a] [--bgcol r,g,b,a] [--disable-editbox] [--fallback <font>.ttf>] <font>.ttf [ptsize] [text]\n"

typedef enum
{
  TextEngineNone,
  TextEngineSurface,
  TextEngineRenderer
} TextEngine;

typedef enum
{
  TextRenderSolid,
  TextRenderShaded,
  TextRenderBlended
} TextRenderMethod;

typedef struct {
  bool done;
  SDL_Window *window;
  SDL_Surface *window_surface;
  SDL_Renderer *renderer;
  TTF_Font *font;
  TTF_Text *caption;
  SDL_Rect captionRect;
  SDL_Texture *message;
  SDL_FRect messageRect;
  TextEngine textEngine;
  SDL_FRect textRect;
  EditBox *edit;
} Scene;

static void DrawScene(Scene *scene)
{
  SDL_Renderer *renderer = scene->renderer;

  /* Clear the background to background color */
  SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
  SDL_RenderClear(renderer);

  if (scene->edit) {
    /* Clear the text rect to light gray */
    SDL_SetRenderDrawColor(renderer, 0xCC, 0xCC, 0xCC, 0xFF);
    SDL_RenderFillRect(renderer, &scene->textRect);

    if (scene->edit->has_focus) {
      SDL_FRect focusRect = scene->textRect;
      focusRect.x -= 1;
      focusRect.y -= 1;
      focusRect.w += 2;
      focusRect.h += 2;
      SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
      SDL_RenderRect(renderer, &focusRect);
    }

    EditBox_Draw(scene->edit);
  }

  switch (scene->textEngine) {
    case TextEngineSurface:
      /* Flush the renderer so we can draw directly to the window surface */
      SDL_FlushRenderer(renderer);
      TTF_DrawSurfaceText(scene->caption, scene->captionRect.x, scene->captionRect.y, scene->window_surface);
      break;
    case TextEngineRenderer:
      TTF_DrawRendererText(scene->caption, (float)scene->captionRect.x, (float)scene->captionRect.y);
      break;
    default:
      SDL_assert(!"Unknown text engine");
      break;
  }

  SDL_RenderTexture(renderer, scene->message, NULL, &scene->messageRect);
  SDL_RenderPresent(renderer);

  if (scene->window_surface) {
    SDL_UpdateWindowSurface(scene->window);
  }
}

static void AdjustTextOffset(TTF_Text *text, int xoffset, int yoffset)
{
  int x, y;

  TTF_GetTextPosition(text, &x, &y);
  x += xoffset;
  y += yoffset;
  TTF_SetTextPosition(text, x, y);
}

static void HandleKeyDown(Scene *scene, SDL_Event *event)
{
  int style, outline;
  float ptsize;

  switch (event->key.key) {
    case SDLK_A:
      /* Cycle alignment */
      switch (TTF_GetFontWrapAlignment(scene->font)) {
        case TTF_HORIZONTAL_ALIGN_LEFT:
          TTF_SetFontWrapAlignment(scene->font, TTF_HORIZONTAL_ALIGN_CENTER);
          break;
        case TTF_HORIZONTAL_ALIGN_CENTER:
          TTF_SetFontWrapAlignment(scene->font, TTF_HORIZONTAL_ALIGN_RIGHT);
          break;
        case TTF_HORIZONTAL_ALIGN_RIGHT:
          TTF_SetFontWrapAlignment(scene->font, TTF_HORIZONTAL_ALIGN_LEFT);
          break;
        default:
          SDL_Log("Unknown wrap alignment: %d", TTF_GetFontWrapAlignment(scene->font));
          break;
      }
      break;

    case SDLK_B:
      /* Toggle bold style */
      style = TTF_GetFontStyle(scene->font);
      if (style & TTF_STYLE_BOLD) {
        style &= ~TTF_STYLE_BOLD;
      } else {
        style |= TTF_STYLE_BOLD;
      }
      TTF_SetFontStyle(scene->font, style);
      break;

    case SDLK_I:
      /* Toggle italic style */
      style = TTF_GetFontStyle(scene->font);
      if (style & TTF_STYLE_ITALIC) {
        style &= ~TTF_STYLE_ITALIC;
      } else {
        style |= TTF_STYLE_ITALIC;
      }
      TTF_SetFontStyle(scene->font, style);
      break;

    case SDLK_O:
      /* Toggle scene->font outline */
      outline = TTF_GetFontOutline(scene->font);
      if (outline) {
        outline = 0;
      } else {
        outline = 1;
      }
      TTF_SetFontOutline(scene->font, outline);
      break;

    case SDLK_R:
      /* Toggle layout direction */
      if (TTF_GetFontDirection(scene->font) == TTF_DIRECTION_INVALID ||
          TTF_GetFontDirection(scene->font) == TTF_DIRECTION_LTR) {
        TTF_SetFontDirection(scene->font, TTF_DIRECTION_RTL);
      } else if (TTF_GetFontDirection(scene->font) == TTF_DIRECTION_RTL) {
        TTF_SetFontDirection(scene->font, TTF_DIRECTION_LTR);
      } else if (TTF_GetFontDirection(scene->font) == TTF_DIRECTION_TTB) {
        TTF_SetFontDirection(scene->font, TTF_DIRECTION_BTT);
      } else if (TTF_GetFontDirection(scene->font) == TTF_DIRECTION_BTT) {
        TTF_SetFontDirection(scene->font, TTF_DIRECTION_TTB);
      }
      break;

    case SDLK_S:
      /* Toggle strike-through style */
      style = TTF_GetFontStyle(scene->font);
      if (style & TTF_STYLE_STRIKETHROUGH) {
        style &= ~TTF_STYLE_STRIKETHROUGH;
      } else {
        style |= TTF_STYLE_STRIKETHROUGH;
      }
      TTF_SetFontStyle(scene->font, style);
      break;

    case SDLK_U:
      /* Toggle underline style */
      style = TTF_GetFontStyle(scene->font);
      if (style & TTF_STYLE_UNDERLINE) {
        style &= ~TTF_STYLE_UNDERLINE;
      } else {
        style |= TTF_STYLE_UNDERLINE;
      }
      TTF_SetFontStyle(scene->font, style);
      break;

    case SDLK_LEFT:
      if (event->key.mod & SDL_KMOD_CTRL) {
        AdjustTextOffset(scene->edit->text, -1, 0);
      }
      break;

    case SDLK_RIGHT:
      if (event->key.mod & SDL_KMOD_CTRL) {
        AdjustTextOffset(scene->edit->text, 1, 0);
      }
      break;

    case SDLK_UP:
      if (event->key.mod & SDL_KMOD_CTRL) {
        AdjustTextOffset(scene->edit->text, 0, -1);
      } else {
        /* Increase font size */
        ptsize = TTF_GetFontSize(scene->font);
        TTF_SetFontSize(scene->font, ptsize + 1.0f);
      }
      break;

    case SDLK_DOWN:
      if (event->key.mod & SDL_KMOD_CTRL) {
        AdjustTextOffset(scene->edit->text, 0, 1);
      } else {
        /* Decrease font size */
        ptsize = TTF_GetFontSize(scene->font);
        TTF_SetFontSize(scene->font, ptsize - 1.0f);
      }
      break;

    case SDLK_COMMA:
      if (event->key.mod & SDL_KMOD_CTRL) {
        TTF_SetFontCharSpacing(scene->font, TTF_GetFontCharSpacing(scene->font) - 1);
      }
      break;

    case SDLK_PERIOD:
      if (event->key.mod & SDL_KMOD_CTRL) {
        TTF_SetFontCharSpacing(scene->font, TTF_GetFontCharSpacing(scene->font) + 1);
      }
      break;

    case SDLK_ESCAPE:
      scene->done = true;
      break;

    default:
      break;
  }
}

static TTF_Font* font;
static SDL_Surface* text;
static Scene scene;
const char* fallback_font_files[MAX_FALLBACKS];
static TTF_Font*   fallback_fonts[MAX_FALLBACKS];
static int num_fallbacks;
static TTF_TextEngine *engine;

void closeWindow() {
  SDL_DestroySurface(text);
  EditBox_Destroy(scene.edit);
  TTF_DestroyText(scene.caption);
  for (int i = 0; i < num_fallbacks; ++i) {
    TTF_CloseFont(fallback_fonts[i]);
  }
  TTF_CloseFont(font);
  switch (scene.textEngine) {
    case TextEngineSurface:
      TTF_DestroySurfaceTextEngine(engine);
      break;
    case TextEngineRenderer:
      TTF_DestroyRendererTextEngine(engine);
      break;
    default:
      break;
  }
  SDL_DestroyTexture(scene.message);
  TTF_Quit();
}

SDL_Window* createWindow(const char *fontpath)
{
  font = NULL;
  text = NULL;
  float ptsize;
  int i;
  SDL_Color white = { 0xFF, 0xFF, 0xFF, SDL_ALPHA_OPAQUE };
  SDL_Color black = { 0x00, 0x00, 0x00, SDL_ALPHA_OPAQUE };
  SDL_Color *forecol;
  SDL_Color *backcol;
  engine = NULL;
  TextRenderMethod rendermethod = TextRenderShaded;
  int renderstyle = TTF_STYLE_NORMAL;
  int outline = 0;
  int hinting = TTF_HINTING_NORMAL;
  int kerning = 1;
  bool wrap = false;
  TTF_HorizontalAlignment align = TTF_HORIZONTAL_ALIGN_LEFT;
  bool editbox = true;
  char *message, string[128];
  num_fallbacks = 0;

  SDL_zero(scene);
  scene.textEngine = TextEngineRenderer;

  SDL_zeroa(fallback_fonts);

  /* Default is black and white */
  forecol = &black;
  backcol = &white;

  /* any fallbacks */

  const char *default_font = fontpath;
  //fallback_font_files[0] = "../ressources/fonts/DejaVuSans-Bold.ttf";
  //fallback_font_files[1] = "../ressources/fonts/DejaVuSans-Oblique.ttf";
  //fallback_font_files[2] = "../ressources/fonts/DejaVuSans-BoldOblique.ttf";
  //num_fallbacks = 3;

  /* engine */
  //scene.textEngine = TextEngineSurface;
  //scene.textEngine = TextEngineRenderer;

  /* render method */
  //rendermethod = TextRenderShaded;
  //rendermethod = TextRenderBlended;
  //rendermethod = TextRenderSolid;

  /* rapport avec les ttf? */
  //renderstyle |= TTF_STYLE_BOLD;
  //renderstyle |= TTF_STYLE_ITALIC;
  //renderstyle |= TTF_STYLE_UNDERLINE;
  //renderstyle |= TTF_STYLE_STRIKETHROUGH;

  /* hinting */
  //hinting = TTF_HINTING_LIGHT;
  //hinting = TTF_HINTING_MONO;
  //hinting = TTF_HINTING_NONE;

  /* no kerning */
  //kerning = 0;

  // wrap = true;

  /* align */
  //align = TTF_HORIZONTAL_ALIGN_LEFT;
  //align = TTF_HORIZONTAL_ALIGN_CENTER;
  //align = TTF_HORIZONTAL_ALIGN_RIGHT;

  /* fg bg */
  //forecol->r = (Uint8)r;
  //forecol->g = (Uint8)g;
  //forecol->b = (Uint8)b;
  //forecol->a = (Uint8)a;

  //backcol->r = (Uint8)r;
  //backcol->g = (Uint8)g;
  //backcol->b = (Uint8)b;
  //backcol->a = (Uint8)a;

  //editbox = false;

  /* Initialize the TTF library */
  if (!TTF_Init()) {
    SDL_Log("Couldn't initialize TTF: %s",SDL_GetError());
    closeWindow();
  }

  /* Open the font file with the requested point size */
  ptsize = DEFAULT_PTSIZE;
  font = TTF_OpenFont(default_font, ptsize);
  if (font == NULL) {
    SDL_Log("Couldn't load %g pt font from %s: %s",
        ptsize, "showfont", SDL_GetError());
    closeWindow();
  }

  TTF_SetFontStyle(font, renderstyle);
  TTF_SetFontOutline(font, outline);
  TTF_SetFontKerning(font, kerning);
  TTF_SetFontHinting(font, hinting);
  TTF_SetFontWrapAlignment(font, align);
  scene.font = font;

  for (i = 0; i < num_fallbacks; ++i) {
    fallback_fonts[i] = TTF_OpenFont(fallback_font_files[i], ptsize);
    if (!fallback_fonts[i]) {
      SDL_Log("Couldn't load %g pt font from %s: %s",
          ptsize, fallback_font_files[i], SDL_GetError());
      closeWindow();
    }
    TTF_AddFallbackFont(font, fallback_fonts[i]);
  }

  /* Create a window */
  scene.window = SDL_CreateWindow("showfont demo", WIDTH, HEIGHT, 0);
  if (!scene.window) {
    SDL_Log("SDL_CreateWindow() failed: %s", SDL_GetError());
    closeWindow();
  }
  if (scene.textEngine == TextEngineSurface) {
    scene.window_surface = SDL_GetWindowSurface(scene.window);
    if (!scene.window_surface) {
      SDL_Log("SDL_CreateWindowSurface() failed: %s", SDL_GetError());
      closeWindow();
    }
    SDL_SetWindowSurfaceVSync(scene.window, 1);

    scene.renderer = SDL_CreateSoftwareRenderer(scene.window_surface);
  } else {
    scene.renderer = SDL_CreateRenderer(scene.window, NULL);
    if (scene.renderer) {
      SDL_SetRenderVSync(scene.renderer, 1);
    }
  }
  if (!scene.renderer) {
    SDL_Log("SDL_CreateRenderer() failed: %s", SDL_GetError());
    closeWindow();
  }

  switch (scene.textEngine) {
    case TextEngineSurface:
      fprintf(stderr, "surface engine\n");
      engine = TTF_CreateSurfaceTextEngine();
      if (!engine) {
        SDL_Log("Couldn't create surface text engine: %s", SDL_GetError());
        closeWindow();
      }
      break;
    case TextEngineRenderer:
      fprintf(stderr, "renderer engine\n");
      engine = TTF_CreateRendererTextEngine(scene.renderer);
      if (!engine) {
        SDL_Log("Couldn't create renderer text engine: %s", SDL_GetError());
        closeWindow();
      }
      break;
    default:
      break;
  }

  /* Show which font file we're looking at */
  SDL_snprintf(string, sizeof(string), "Font file: %s", "showfont");  /* possible overflow */
  scene.caption = TTF_CreateText(engine, font, string, 0);
  TTF_SetTextColor(scene.caption, forecol->r, forecol->g, forecol->b, forecol->a);
  scene.captionRect.x = 4;
  scene.captionRect.y = 4;
  TTF_GetTextSize(scene.caption, &scene.captionRect.w, &scene.captionRect.h);

  /* Render and center the message */
  message = DEFAULT_TEXT;

  switch (rendermethod) {
    case TextRenderSolid:
      if (wrap) {
        text = TTF_RenderText_Solid_Wrapped(font, message, 0, *forecol, 0);
      } else {
        text = TTF_RenderText_Solid(font, message, 0, *forecol);
      }
      break;
    case TextRenderShaded:
      if (wrap) {
        text = TTF_RenderText_Shaded_Wrapped(font, message, 0, *forecol, *backcol, 0);
      } else {
        text = TTF_RenderText_Shaded(font, message, 0, *forecol, *backcol);
      }
      break;
    case TextRenderBlended:
      if (wrap) {
        text = TTF_RenderText_Blended_Wrapped(font, message, 0, *forecol, 0);
      } else {
        text = TTF_RenderText_Blended(font, message, 0, *forecol);
      }
      break;
  }
  if (text == NULL) {
    SDL_Log("Couldn't render text: %s", SDL_GetError());
    abort();
  }
  scene.messageRect.x = (float)((WIDTH - text->w)/2);
  scene.messageRect.y = (float)((HEIGHT - text->h)/2);
  scene.messageRect.w = (float)text->w;
  scene.messageRect.h = (float)text->h;
  scene.message = SDL_CreateTextureFromSurface(scene.renderer, text);
  SDL_Log("Font is generally %d big, and string is %d big",
      TTF_GetFontHeight(font), text->h);

  if (editbox) {
    scene.textRect.x = 8.0f;
    scene.textRect.y = scene.captionRect.y + scene.captionRect.h + 4.0f;
    scene.textRect.w = WIDTH / 2 - scene.textRect.x * 2;
    scene.textRect.h = scene.messageRect.y - scene.textRect.y - 16.0f;

    SDL_FRect editRect = scene.textRect;
    editRect.x += 4.0f;
    editRect.y += 4.0f;
    editRect.w -= 8.0f;
    editRect.w -= 8.0f;
    scene.edit = EditBox_Create(scene.window, scene.renderer, engine, font, &editRect);
    if (scene.edit) {
      TTF_SetTextColor(scene.edit->text, forecol->r, forecol->g, forecol->b, forecol->a);

      EditBox_Insert(scene.edit, message);
    }
  }
   /* ---- */
  fprintf(stderr, "cfg ptsize:%f\n", ptsize);
  fprintf(stderr, "cfg rendermethod:%i\n", rendermethod);
  fprintf(stderr, "cfg renderstyle:%i\n", renderstyle);
  fprintf(stderr, "cfg renderoutline:%i\n", outline);
  fprintf(stderr, "cfg renderhinting:%i\n", hinting);
  fprintf(stderr, "cfg renderkerning:%i\n", kerning);
  fprintf(stderr, "cfg wrap:%i\n", wrap);
  fprintf(stderr, "cfg align:%i\n", align);
  fprintf(stderr, "cfg editbox:%i\n", editbox);
  return scene.window;
}

SDL_Event demoevt;
void handleEvent() {
  /* Wait for a keystroke, and blit text on mouse press */
      SDL_ConvertEventToRenderCoordinates(scene.renderer, &demoevt);

      switch (demoevt.type) {
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
          if (!EditBox_HandleEvent(scene.edit, &demoevt)) {
            scene.messageRect.x = (demoevt.button.x - text->w/2);
            scene.messageRect.y = (demoevt.button.y - text->h/2);
            scene.messageRect.w = (float)text->w;
            scene.messageRect.h = (float)text->h;
          }
          break;

        case SDL_EVENT_KEY_DOWN:
          if (!EditBox_HandleEvent(scene.edit, &demoevt)) {
            HandleKeyDown(&scene, &demoevt);
          }
          break;

        case SDL_EVENT_QUIT:
          scene.done = true;
          break;

        default:
          EditBox_HandleEvent(scene.edit, &demoevt);
          break;
      }
    DrawScene(&scene);
}


/*
int _main(int argc, char *argv[]) {
  SDL_Window* _result = createWindow();
  SDL_Event event;
  while (!scene.done) {
    while (SDL_PollEvent(&event)) {
      handleEvent(event);
    }
  }

  closeWindow();
  SDL_Quit();
  return 0;
}
*/
