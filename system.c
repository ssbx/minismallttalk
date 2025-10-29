/*
 * Copyright © 2025 Sébastien Serre. All rights reserved.
 *
 * This file is part of Minist8.
 *
 * Minist8 is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Minist8 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Minist8. If not, see <https://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include <SDL3/SDL.h>


#include "minist8.h"
#include "interpreter.h"
#include "system.h"

extern Options  G_Options;
typedef uint16_t Pixel;
typedef struct Queue_t {
    uint16_t       *queue;
    int             size;
    int             used;
    int             tail;
    int             head;
} Queue_t;
static void     ___q_init(Queue_t *);
static void     ___q_enqueue(Queue_t *, uint16_t v);
static bool     ___q_dequeue(Queue_t *, uint16_t * v);
/*
 * static int ___q_size(Queue_t*);
 */
static void     ___q_clear(Queue_t *);
static void     __QueueInputSemaphoreTimeWord();
static void     __QueueInputSemaphoreWord(uint16_t word);
static void     __QueueInputSemaphoreWord2(uint16_t type, uint16_t parameter);

static void     __HandleKeyboardEvent(const SDL_KeyboardEvent * keyevent);
static void     __HandleTextEvent(const SDL_TextInputEvent * evt);
static void     __HandleMouseButtonEvent(const SDL_MouseButtonEvent * mouse);
static void     __HandleMouseMovementEvent(const SDL_MouseMotionEvent * motion);
static bool     __InitDisplay(int scale, bool vsync);
static void     __CloseDisplay();
static char    *__PathForFile(const char *name);
static SDL_Cursor *__CreateCursor(const Uint8 * cursor_bits);
static void     __SetCursorImage(uint16_t * image);
static void     __SetCursorLocation(int x, int y);
static void     __GetCursorLocation(int *x, int *y);
static void     __InitializeTexture();
static bool     __SetDisplaySize(int width, int height);
static void     __DisplayChanged(int x, int y, int width, int height);
static void     __Render();

static Uint32   __msDelay = 0;
static Queue_t  __inputSemaphoreQueue;
static int      __eventCount;
static int      __inputSemaphore;
static int      __stdinInputSemaphore;
static int      __scheduledSemaphore;
static uint32_t __lastEventTime;
static uint32_t __scheduledTime;
static SDL_Window *__sdlWindow;
static SDL_Renderer *__sdlRenderer;
static SDL_Texture *__sdlTexture;
static SDL_Cursor *__sdlCursor;
static SDL_Rect __sdlDirtyRect;
static bool     __useVsync;
static bool     __textureNeedsUpdate;
static int      __displayScale;
static int      __displayWidth;
static int      __displayHeight;
static bool     __quitSignalled;

/*****************************************************************************/
/*
 * input/output/file routines
 */
/*****************************************************************************/
static char    *
__PathForFile(const char *name)
{
    size_t          l;
    char           *path;

    l = strlen(G_Options.rootDir) + 1 + strlen(name) + 1;
    path = malloc(sizeof(char) * l);
    sprintf(path, "%s/%s", G_Options.rootDir, name);
    return path;
}

/*
 * File oriented operations
 */
int
System_OpenFile(const char *name)
{
    char           *path;
    int             fd;

    path = __PathForFile(name);
    fd = open(path, O_RDWR);
    free(path);
    return fd;
}

int
System_CreateFile(const char *name)
{
    char           *path;
    int             fd;

    path = __PathForFile(name);
    fd = open(path, O_RDWR | O_CREAT | O_TRUNC, 0644);
    free(path);
    return fd;
}

int
System_CloseFile(int fd)
{
    return close(fd);
}

int
System_ReadFile(int fd, char *buffer, int bytes)
{
    return (int) read(fd, buffer, bytes);
}

int
System_WriteFile(int fd, const char *buffer, int bytes)
{
    return (int) write(fd, buffer, bytes);
}

bool
System_TruncateFileTo(int fd, int length)
{
    return ftruncate(fd, length) != -1;
}

int
System_FileSize(int fd)
{
    struct stat     s;
    if (fstat(fd, &s) != -1)
        return (int) s.st_size;
    return -1;
}

bool
System_FlushFile(int fd)
{
    return true;
}

bool
System_FileIsDirectory(const char *name)
{
    char           *path;
    struct stat     sb;
    int             ret;

    path = __PathForFile(name);
    ret = stat(path, &sb);
    free(path);
    if (ret == 0)
        if ((sb.st_mode & S_IFMT) == S_IFDIR)
            return 1;
    return 0;
}

/*
 * Directory orientated operations
 */
char          **
System_ListDir(int *num)
{
    DIR            *dir;
    struct dirent  *entry;
    long            n;
    char          **dirlist;
    int             index;

    dir = opendir(G_Options.rootDir);
    n = 0;
    while ((entry = readdir(dir))) {
        if (entry->d_name[0] != '.' && !System_FileIsDirectory(entry->d_name)) {
            n++;
        }
    }
    dirlist = malloc(sizeof(char *) * n);
    seekdir(dir, 0);
    index = 0;
    while ((entry = readdir(dir))) {
        if (entry->d_name[0] != '.' && !System_FileIsDirectory(entry->d_name)) {
            dirlist[index++] = strdup(entry->d_name);
        }
    }
    *num = n;
    closedir(dir);
    return dirlist;
}

void
System_FreeListDir(char **lsdir, int num)
{
    int             i;

    for (i = 0; i < num; i++) {
        free(lsdir[i]);
    }
    free(lsdir);
}

bool
System_RenameFile(const char *old_name, const char *new_name)
{
    char           *old_path = __PathForFile(old_name);
    char           *new_path = __PathForFile(new_name);
    int             ret = rename(old_path, new_path) != -1;
    free(old_path);
    free(new_path);
    return ret;
}

bool
System_DeleteFile(const char *file_name)
{
    char           *path = __PathForFile(file_name);
    int             ret = unlink(path) != -1;
    free(path);
    return ret;
}

int
System_SeekFileTo(int fd, int position)
{
    return (int) lseek(fd, position, SEEK_SET);
}

int
System_TellFile(int fd)
{
    return (int) lseek(fd, 0, SEEK_CUR);
}

/*
 * Error handling
 */
const int
System_LastError()
{
    return errno;
}

const char     *
System_ErrorText(int code)
{
    return strerror(code);
}

#define BUFF_SIZE 500
int             buffer_index = 0;
char            buffer[BUFF_SIZE];
int
System_ReadStdin()
{
    char            c;
    while ((c = getc(stdin)) != EOF) {
        buffer[buffer_index++] = c;
    }

    /*
     * remove newline character
     */
    if (buffer_index) {
        buffer[--buffer_index] = '\0';
    }
    return buffer_index;
}

const char     *
System_GetStdinLine()
{
    buffer_index = 0;
    return buffer;
}

/*****************************************************************************/
/*
 * SDL Display routines
 */
/*****************************************************************************/
static bool
__InitDisplay(int scale, bool vsync)
{

    if (SDL_Init(SDL_INIT_VIDEO) == false) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't initialize SDL: %s", SDL_GetError());
        return false;
    }
    __sdlWindow = NULL;
    __sdlRenderer = NULL;
    __sdlTexture = NULL;
    __sdlCursor = NULL;
    __useVsync = vsync;
    __textureNeedsUpdate = false;
    __displayScale = scale;
    __displayWidth = 0;
    __displayHeight = 0;

    return true;
}

static void
__CloseDisplay()
{
    if (__sdlCursor)
        SDL_DestroyCursor(__sdlCursor);
    if (__sdlTexture)
        SDL_DestroyTexture(__sdlTexture);
    if (__sdlRenderer)
        SDL_DestroyRenderer(__sdlRenderer);
    if (__sdlWindow) {
        SDL_StopTextInput(__sdlWindow);
        SDL_DestroyWindow(__sdlWindow);
    }
    __sdlCursor = NULL;
    __sdlTexture = NULL;
    __sdlRenderer = NULL;
    __sdlWindow = NULL;

}

static SDL_Cursor *
__CreateCursor(const Uint8 * cursor_bits)
{
    /* *INDENT-OFF* */
    /* Maps a nibble to a byte where each bit is repeated */
    /* e.g. 1010 -> 11001100 */
    static int      expandedNibbleToByte[] = {
        0x00,            /* 0000 */
        0x03,            /* 0001 */
        0x0c,            /* 0010 */
        0x0f,            /* 0011 */
        0x30,            /* 0100 */
        0x33,            /* 0101 */
        0x3c,            /* 0110 */
        0x3f,            /* 0111 */
        0xc0,            /* 1000 */
        0xc3,            /* 1001 */
        0xcc,            /* 1010 */
        0xcf,            /* 1011 */
        0xf0,            /* 1100 */
        0xf3,            /* 1101 */
        0xfc,            /* 1110 */
        0xff             /* 1111 */
    };
    /* *INDENT-ON* */

    SDL_Cursor     *new_cursor = 0;

    if (__displayScale == 1) {
        new_cursor = SDL_CreateCursor((const Uint8 *) cursor_bits,
                                      (const Uint8 *) cursor_bits, 16, 16, 0,
                                      0);

    } else if (__displayScale == 2) {
        uint8_t         image[128];
        uint8_t        *src = (uint8_t *) cursor_bits;
        int             dest = 0;

        int             h;
        /*
         * 4 bytes (32-bits) x 32 rows
         */
        for (h = 0; h < 16; h++) {
            image[dest] = expandedNibbleToByte[*(src) >> 4];
            image[dest + 1] = expandedNibbleToByte[*(src) & 0xf];
            image[dest + 2] = expandedNibbleToByte[*(src + 1) >> 4];
            image[dest + 3] = expandedNibbleToByte[*(src + 1) & 0xf];

            image[dest + 4] = image[dest];
            image[dest + 5] = image[dest + 1];
            image[dest + 6] = image[dest + 2];
            image[dest + 7] = image[dest + 3];
            dest += 8;
            src += 2;
        }

        new_cursor =
            SDL_CreateCursor((const Uint8 *) image, (const Uint8 *) image, 32,
                             32, 0, 0);
    }
    if (!new_cursor) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Create Cursor failed: %s",
                     SDL_GetError());

    }

    return new_cursor;
}

/*
 * Set the cursor image
 */
/*
 * (a 16 word form)
 */
static void
__SetCursorImage(uint16_t * image)
{
    uint16_t        cursor_bits[16];
    SDL_Cursor     *old_cursor;

    /*
     * SDL uses a MSB format, so swap the bytes
     */
    int             i;
    for (i = 0; i < 16; i++) {
        cursor_bits[i] = ((image[i] & 0xff) << 8) | (image[i] >> 8);
    }

    old_cursor = __sdlCursor;

    __sdlCursor = __CreateCursor((const Uint8 *) cursor_bits);

    SDL_SetCursor(__sdlCursor);

    if (old_cursor) {
        SDL_DestroyCursor(old_cursor);
    }
}

static void
__SetCursorLocation(int x, int y)
{
    SDL_WarpMouseInWindow(__sdlWindow, x * __displayScale, y * __displayScale);
}

static void
__GetCursorLocation(int *x, int *y)
{
    float           xf;
    float           yf;

    SDL_GetMouseState(&xf, &yf);
    *x = xf / __displayScale;
    *y = yf / __displayScale;
}

static void
__InitializeTexture()
{
    uint8_t        *dest_row;
    int             dest_pitch;
    int             code;
    Pixel          *dest_pixel;
    int             h;
    int             i;

    code = SDL_LockTexture(__sdlTexture, 0, (void **) &dest_row, &dest_pitch);
    if (code < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't LOCK SDL: %s",
                     SDL_GetError());
        return;
    }
    /*
     * There may be many frames before Smalltalk renders, so initialize the
     */
    /*
     * screen texture
     */
    /*
     * with something that looks like the Smalltalk desktop pattern
     */
    for (h = 0; h < __displayHeight; h++) {
        dest_pixel = (Pixel *) dest_row;

        for (i = 0; i < __displayWidth; i++) {
            *dest_pixel++ = (h & 1) ^ (i & 1) ? 0 : ~0;
        }
        dest_row += dest_pitch;
    }
    SDL_UnlockTexture(__sdlTexture);
}

static bool
__SetDisplaySize(int width, int height)
{

    SDL_PropertiesID props;
    if (__displayWidth != width || __displayHeight != height) {
        __displayWidth = width;
        __displayHeight = height;
        __sdlDirtyRect.x = 0;
        __sdlDirtyRect.y = 0;
        __sdlDirtyRect.w = width;
        __sdlDirtyRect.h = height;

        if (__sdlWindow) {
            SDL_SetWindowSize(__sdlWindow, __displayScale * __displayWidth,
                              __displayScale * __displayHeight);

            SDL_DestroyTexture(__sdlTexture);
        } else {
            props = SDL_CreateProperties();
            SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING,
                                  "minist8");
            SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER,
                                  __displayScale * __displayWidth);
            SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER,
                                  __displayScale * __displayHeight);
            SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_FLAGS_NUMBER,
                                  0);
            __sdlWindow = SDL_CreateWindowWithProperties(props);
            SDL_DestroyProperties(props);

            props = SDL_CreateProperties();
            SDL_SetPointerProperty(props,
                                   SDL_PROP_RENDERER_CREATE_WINDOW_POINTER,
                                   __sdlWindow);
            SDL_SetBooleanProperty(props,
                                   SDL_PROP_RENDERER_CREATE_PRESENT_VSYNC_NUMBER,
                                   __useVsync);
            __sdlRenderer = SDL_CreateRendererWithProperties(props);
            SDL_DestroyProperties(props);

            SDL_StartTextInput(__sdlWindow);
        }

        __sdlTexture = SDL_CreateTexture(__sdlRenderer, SDL_PIXELFORMAT_RGB565,
                                         SDL_TEXTUREACCESS_STREAMING,
                                         __displayWidth, __displayHeight);
        __InitializeTexture();
    }

    return true;
}

static void
__DisplayChanged(int x, int y, int width, int height)
{
    __textureNeedsUpdate = true;
    assert(x >= 0 && x < __displayWidth);
    assert(y >= 0 && y < __displayHeight);
    assert(x + width <= __displayWidth);
    assert(y + height <= __displayHeight);

    if (SDL_RectEmpty(&__sdlDirtyRect)) {
        __sdlDirtyRect.x = x;
        __sdlDirtyRect.y = y;
        __sdlDirtyRect.w = width;
        __sdlDirtyRect.h = height;
    } else {
        SDL_Rect        update_rect;
        update_rect.x = x;
        update_rect.y = y;
        update_rect.w = width;
        update_rect.h = height;
        SDL_GetRectUnion(&__sdlDirtyRect, &update_rect, &__sdlDirtyRect);
    }
}

static void
__Render()
{
    if (__sdlRenderer) {
        if (__sdlTexture)
            SDL_RenderTexture(__sdlRenderer, __sdlTexture, NULL, NULL);
        SDL_RenderPresent(__sdlRenderer);
        __sdlDirtyRect.x = 0;
        __sdlDirtyRect.y = 0;
        __sdlDirtyRect.w = 0;
        __sdlDirtyRect.h = 0;
    }
}

static void
__ExpandPixel(Pixel * destPixel, uint16_t srcWord, int srcBit)
{
    *destPixel = -((srcWord & (1 << srcBit)) == 0);
}

static void
__UpdateTexture()
{
    SDL_Rect        update_rect;
    int             source_word_left;
    int             source_word_right;
    int             display_width_words;
    int             displayBitmap;
    int             source_index_row;
    int             update_word_width;
    int             dest_pitch;
    int             code;
    uint8_t        *dest_row;
    Pixel          *dest_pixel;
    uint8_t        *pixels;
    int             source_index;
    uint16_t        source_pixel;
    int             i;
    int             h;

    source_word_left = __sdlDirtyRect.x / 16;
    source_word_right = (__sdlDirtyRect.x + __sdlDirtyRect.w - 1) / 16;
    display_width_words = (__displayWidth + 15) / 16;
    source_index_row =
        source_word_left + (__sdlDirtyRect.y * display_width_words);
    update_word_width = source_word_right - source_word_left + 1;

    /*
     * We transfer pixels in groups of WORDS from the display form,
     */
    /*
     * so we need to set texture update rectangle so the left and right edges
     */
    /*
     * are on
     */
    /*
     * a word boundary
     */
    update_rect.x = source_word_left * 16;
    update_rect.y = __sdlDirtyRect.y;
    update_rect.w = update_word_width * 16;
    update_rect.h = __sdlDirtyRect.h;

    displayBitmap = Interp_GetDisplayBits(__displayWidth, __displayHeight);

    if (displayBitmap == 0)
        return;

    code = SDL_LockTexture(__sdlTexture, &update_rect,
                           (void **) &pixels, &dest_pitch);

    if (code < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't LOCK SDL: %s",
                     SDL_GetError());
        return;
    }

    dest_row = pixels;

    for (h = 0; h < update_rect.h; h++) {
        dest_pixel = (Pixel *) dest_row;
        source_index = source_index_row;
        for (i = 0; i < update_word_width; i++) {
            source_pixel = Interp_FetchWord_OfDisplayBits(source_index,
                                                          displayBitmap);
            __ExpandPixel(dest_pixel++, source_pixel, 15);
            __ExpandPixel(dest_pixel++, source_pixel, 14);
            __ExpandPixel(dest_pixel++, source_pixel, 13);
            __ExpandPixel(dest_pixel++, source_pixel, 12);
            __ExpandPixel(dest_pixel++, source_pixel, 11);
            __ExpandPixel(dest_pixel++, source_pixel, 10);
            __ExpandPixel(dest_pixel++, source_pixel, 9);
            __ExpandPixel(dest_pixel++, source_pixel, 8);
            __ExpandPixel(dest_pixel++, source_pixel, 7);
            __ExpandPixel(dest_pixel++, source_pixel, 6);
            __ExpandPixel(dest_pixel++, source_pixel, 5);
            __ExpandPixel(dest_pixel++, source_pixel, 4);
            __ExpandPixel(dest_pixel++, source_pixel, 3);
            __ExpandPixel(dest_pixel++, source_pixel, 2);
            __ExpandPixel(dest_pixel++, source_pixel, 1);
            __ExpandPixel(dest_pixel++, source_pixel, 0);

            source_index++;
        }
        dest_row += dest_pitch;
        source_index_row += display_width_words;
    }

    SDL_UnlockTexture(__sdlTexture);
}

/*****************************************************************************/
/*
 * SDL Events Queuing
 */
/*****************************************************************************/
/*
 * queue imp
 */
#define QSIZE_INITIAL 16
static int
___q_step(Queue_t * q, int v)
{
    if (v++ < q->size)
        return v;
    else
        return 0;
}

static void
___q_maybe_realloc(Queue_t * q)
{
    int             new_size;
    int             idx;
    int             i;
    uint16_t       *new_queue;


    if (q->used == q->size) {
        new_size = q->size * 2;
        new_queue = calloc(sizeof(uint16_t), new_size);
        idx = 0;
        for (i = q->head; i < q->size; i++)
            new_queue[idx++] = q->queue[i];
        for (i = 0; i < q->head; i++)
            new_queue[idx++] = q->queue[i];
        free(q->queue);
        q->queue = new_queue;
        q->head = 0;
        q->tail = q->used;
        q->size = new_size;
    }
}

static void
___q_init(Queue_t * q)
{
    q->queue = calloc(sizeof(uint16_t), QSIZE_INITIAL);
    q->used = 0;
    q->size = QSIZE_INITIAL;
    q->tail = 0;
    q->head = 0;
}

static void
___q_enqueue(Queue_t * q, uint16_t v)
{
    ___q_maybe_realloc(q);
    q->queue[q->tail] = v;
    q->used++;
    q->tail = ___q_step(q, q->tail);
}

static bool
___q_dequeue(Queue_t * q, uint16_t * v)
{
    if (q->used == 0) {
        return false;
    } else {
        *v = q->queue[q->head];
        q->queue[q->head] = 0;
        q->used--;
        q->head = ___q_step(q, q->head);
        return true;
    }
}

/*
 * int ___q_size(Queue_t* q) { return q->used; }
 */

static void
___q_clear(Queue_t * q)
{
    if (!q->queue)
        return;
    free(q->queue);
    q->queue = NULL;
}

/*
 * input_semaphore queue routines
 */
static void
__QueueInputSemaphoreWord(uint16_t word)
{
    assert(__inputSemaphore);
    ___q_enqueue(&__inputSemaphoreQueue, word);
    Interp_AsynchronousSignal(__inputSemaphore);
}

static void
__QueueInputSemaphoreWord2(uint16_t type, uint16_t parameter)
{
    __QueueInputSemaphoreWord(((type & 0xf) << 12) | (parameter & 0xfff));
}

static void
__QueueInputSemaphoreTimeWord()
{
    uint32_t        delta_time;
    uint32_t        now = System_GetMsClock();
    if (__eventCount++ == 0) {
        delta_time = 0;
    } else {
        delta_time = now - __lastEventTime;
    }

    if (delta_time <= 4095) {
        /*
         * can fit in 12 bits
         */
        __QueueInputSemaphoreWord2(0, delta_time);
    } else {
        uint32_t        abs_time = System_GetSmalltalkEpochTime();
        /*
         * too large, use type 5 with absolute time
         */
        __QueueInputSemaphoreWord2(5, 0);       /* parameter is ignored */
        __QueueInputSemaphoreWord((abs_time >> 16) & 0xffff);   /* high word */
        /*
         * first
         */
        __QueueInputSemaphoreWord(abs_time & 0xffff);   /* low word next */

    }

    __lastEventTime = now;
}

/*****************************************************************************/
/*
 * SDL Events Handling
 */
/*****************************************************************************/
static void
__HandleKeyboardEvent(const SDL_KeyboardEvent * keyevent)
{
    uint16_t        type = keyevent->type == SDL_EVENT_KEY_DOWN ? 3 : 4;
    SDL_Keymod      mod = SDL_GetModState();
    switch (keyevent->scancode) {
    case SDL_SCANCODE_LCTRL:
    case SDL_SCANCODE_RCTRL:
        __QueueInputSemaphoreTimeWord();
        __QueueInputSemaphoreWord2(type, 138);
        return;
    case SDL_SCANCODE_CAPSLOCK:
        __QueueInputSemaphoreTimeWord();
        __QueueInputSemaphoreWord2(type, 139);
        return;
    case SDL_SCANCODE_DELETE:
        __QueueInputSemaphoreTimeWord();
        __QueueInputSemaphoreWord2(type, 127);
        return;
    case SDL_SCANCODE_RETURN:
        __QueueInputSemaphoreTimeWord();
        __QueueInputSemaphoreWord2(type, 13);
        return;
    case SDL_SCANCODE_BACKSPACE:
        __QueueInputSemaphoreTimeWord();
        __QueueInputSemaphoreWord2(type, 8);
        return;
    case SDL_SCANCODE_ESCAPE:
        __QueueInputSemaphoreTimeWord();
        __QueueInputSemaphoreWord2(type, 27);
        return;
    default:
        if (mod & SDL_KMOD_CTRL) {
            if (keyevent->key < 128) {
                if (keyevent->type == SDL_EVENT_KEY_DOWN) {
                    uint16_t        param = keyevent->key & 0x7f;
                    fprintf(stderr, "queue\n");
                    __QueueInputSemaphoreTimeWord();
                    __QueueInputSemaphoreWord2(3, param);
                    __QueueInputSemaphoreWord2(4, param);
                }
            }
        }
        break;
    }
}

static void
__HandleTextEvent(const SDL_TextInputEvent * evt)
{
    int             len;
    char            c;
    int             i;

    len = strlen(evt->text);
    for (i = 0; i < len; i++) {
        c = evt->text[i];
        if (c <= 0 || c >= 128) {
            fprintf(stderr, "unhandled char %c in text %s\n", c, evt->text);
            return;
        }
    }
    for (i = 0; i < len; i++) {
        c = evt->text[i];
        __QueueInputSemaphoreTimeWord();
        __QueueInputSemaphoreWord2(3, (uint16_t) c);
        __QueueInputSemaphoreWord2(4, (uint16_t) c);
    }
}

static void
__HandleMouseButtonEvent(const SDL_MouseButtonEvent * mouse)
{
    /*
     * The bluebook got these wrong!
     */
    const int       RedButton = 130;    /* select */
    const int       YellowButton = 129; /* doit etc. */
    const int       BlueButton = 128;   /* frame, close */
    unsigned        mods;
    int             smalltalk_button = 0;
    int             button_index;
    static unsigned button_down_mods[3] = { 0 };        /* modifier state at */
    /*
     * button down
     */
    switch (mouse->button) {
    case SDL_BUTTON_LEFT:
        smalltalk_button = RedButton;
        button_index = 0;
        break;
    case SDL_BUTTON_MIDDLE:
        smalltalk_button = BlueButton;
        button_index = 1;
        break;
    case SDL_BUTTON_RIGHT:
        smalltalk_button = YellowButton;
        button_index = 2;
        break;
    default:
        return;
    }

    if (mouse->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        /*
         * Save mod state when the button went down
         */
        /*
         * when the button is released we will use these rather than active one
         */
        mods = SDL_GetModState();
        button_down_mods[button_index] = mods;
    } else
        mods = button_down_mods[button_index];

    if (G_Options.threeButtons) {
        /*
         * Real 3 button mouse
         */
        switch (mouse->button) {
        case SDL_BUTTON_LEFT:
            smalltalk_button = RedButton;
            break;
        case SDL_BUTTON_MIDDLE:
            smalltalk_button = YellowButton;
            break;
        case SDL_BUTTON_RIGHT:
            smalltalk_button = BlueButton;
            break;
        default:
            return;
        }
    } else {
        const Uint32    BlueFlags = SDL_KMOD_RALT | SDL_KMOD_LALT;
        /*
         * Left = Red Right/Ctrl+Left = Yellow Alt+Left(win+linux)
         * /Command+Left(mac) = Blue
         */
        switch (mouse->button) {
        case SDL_BUTTON_LEFT:
            if (mods & BlueFlags)
                smalltalk_button = BlueButton;
            else if (mods & (SDL_KMOD_RCTRL | SDL_KMOD_LCTRL))
                smalltalk_button = YellowButton;
            else
                smalltalk_button = RedButton;
            break;
        case SDL_BUTTON_RIGHT:
            smalltalk_button = YellowButton;
            break;
        default:
            return;             /* Don't care about this button */
        }
    }

    if (mouse->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        __QueueInputSemaphoreTimeWord();
        __QueueInputSemaphoreWord2(3, smalltalk_button);
    } else if (mouse->type == SDL_EVENT_MOUSE_BUTTON_UP) {
        button_down_mods[button_index] = 0;
        __QueueInputSemaphoreTimeWord();
        __QueueInputSemaphoreWord2(4, smalltalk_button);
    }
}

static void
__HandleMouseMovementEvent(const SDL_MouseMotionEvent * motion)
{
    __QueueInputSemaphoreTimeWord();
    __QueueInputSemaphoreWord2(1, (uint16_t) motion->x);
    __QueueInputSemaphoreTimeWord();
    __QueueInputSemaphoreWord2(1, (uint16_t) motion->y);
}
/*****************************************************************************/
/*
 * main calls
 */
/*****************************************************************************/


int
System_GetInputSemaphore()
{
    return __inputSemaphore;
}

int
System_GetStdinInputSemaphore()
{
    return __stdinInputSemaphore;
}

void
System_CheckScheduledSemaphore()
{
    if (__scheduledSemaphore && (SDL_GetTicks() <= __scheduledTime)) {
        Interp_AsynchronousSignal(__scheduledSemaphore);
        __scheduledSemaphore = 0;
    }
}

void
System_ProcessEvents()
{
    SDL_Event       event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
            __quitSignalled = true;
            break;
        }
        if (__inputSemaphore) {
            switch (event.type) {
            case SDL_EVENT_TEXT_INPUT:
                __HandleTextEvent(&event.text);
                break;
            case SDL_EVENT_KEY_UP:
            case SDL_EVENT_KEY_DOWN:
                __HandleKeyboardEvent(&event.key);
                break;
            case SDL_EVENT_MOUSE_BUTTON_UP:
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                __HandleMouseButtonEvent(&event.button);
                break;
            case SDL_EVENT_MOUSE_MOTION:
                __HandleMouseMovementEvent(&event.motion);
                break;
            default:{
                }
            }
        }
    }
}

bool
System_QuitSignalled()
{
    return __quitSignalled;
}

void
System_UpdateScreen()
{
    if (__textureNeedsUpdate) {
        __UpdateTexture();
        __textureNeedsUpdate = false;
        __Render();
    }
}


/*****************************************************************************/
/*
 * Interpreter calls
 */
/*****************************************************************************/


void
System_SetInputSemaphore(int semaphore)
{
    __inputSemaphore = semaphore;
}

void
System_SetStdinInputSemaphore(int semaphore)
{
    __stdinInputSemaphore = semaphore;
}


#define TIME_OFFSET 2177452800
uint32_t
System_GetSmalltalkEpochTime()
{
    time_t          unix_epoch_time = time(0);
    return (uint32_t) unix_epoch_time + TIME_OFFSET;
}

/*
 * the number of milliseconds since the millisecond clock was
 */
/*
 * last reset or rolled over (a 32-bit unsigned number)
 */
uint32_t
System_GetMsClock()
{
    return SDL_GetTicks();
}

/*
 * Schedule a semaphore to be signaled at a time. Only one outstanding
 */
/*
 * request may be scheduled at anytime. When called any outstanding
 */
/*
 * request will be replaced (or canceled if semaphore is 0).
 */
/*
 * Will signal immediate if scheduled time has passed.
 */
void
System_SignalSemaphoreAt(int semaphore, uint32_t msClockTime)
{
    __scheduledSemaphore = semaphore;
    __scheduledTime = msClockTime;
    fprintf(stderr, "signal at\n");
    if (semaphore) {
        /*
         * Just in case the time passed
         */
        System_CheckScheduledSemaphore();
    }
}

/*
 * Set the cursor image
 */
/*
 * (a 16 word form)
 */
void
System_SetCursorImage(uint16_t * image)
{
    __SetCursorImage(image);
}

/*
 * Set the mouse cursor location
 */
void
System_SetCursorLocation(int x, int y)
{
    __SetCursorLocation(x, y);
}

void
System_GetCursorLocation(int *x, int *y)
{
    __GetCursorLocation(x, y);
}

void
System_SetLinkCursor(bool link)
{
}

bool
System_SetDisplaySize(int width, int height)
{
    return __SetDisplaySize(width, height);
}

void
System_DisplayChanged(int x, int y, int width, int height)
{
    __DisplayChanged(x, y, width, height);
}

bool
System_NextInputQueueWord(uint16_t * word)
{
    return ___q_dequeue(&__inputSemaphoreQueue, word);
}

void
System_Error(const char *message)
{
    fprintf(stderr, "%s", message);
    abort();
}

/*
 * lifetime
 */
void
System_SignalQuit()
{
    __quitSignalled = true;
}

void
System_ExitToDebugger()
{
    abort();
}

const char     *
System_GetImageName()
{
    return G_Options.imgName;
}

void
System_SetImageName(const char *new_name)
{
    free(G_Options.imgName);
    G_Options.imgName = strdup(new_name);
}

bool
System_Init()
{
    int             flags;
    ___q_init(&__inputSemaphoreQueue);
    __eventCount = 0;
    __inputSemaphore = 0;
    __stdinInputSemaphore = 0;
    __scheduledSemaphore = 0;
    __scheduledTime = 0;
    __lastEventTime = 0;
    __quitSignalled = false;
    __msDelay = (Uint32) G_Options.noVsyncDelay;

    flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    return __InitDisplay(G_Options.displayScale, G_Options.useVsync);
}

void
System_Quit()
{
    __CloseDisplay();
    ___q_clear(&__inputSemaphoreQueue);
}
