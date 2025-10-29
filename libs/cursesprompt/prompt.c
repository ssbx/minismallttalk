#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <wchar.h>
#include <ncursesw/term.h>
#include <ncursesw/unctrl.h>
#include <string.h>
#include <ncurses.h>

#define MY_KEYS (KEY_MAX + 1)

void
test_ncurses(int argc GCC_UNUSED, char *argv[]GCC_UNUSED)
{
  int n;
  int ch;

  if (newterm(0, stdout, stdin) == 0) {
    fprintf(stderr, "Cannot initialize terminal\n");
    exit_terminfo(EXIT_FAILURE);
  }
  (void) cbreak();		/* take input chars one at a time, no wait for \n */
  (void) noecho();		/* don't echo input */

  scrollok(stdscr, TRUE);
  keypad(stdscr, TRUE);
  move(0, 0);

  /* we do the define_key() calls after keypad(), since the first call to
   * keypad() initializes the corresponding data.
   */
  for (n = 0; n < 255; ++n) {
    char temp[10];
    define_key(temp, n + MY_KEYS);
  }
  for (n = KEY_MIN; n < KEY_MAX; ++n) {
    char *value;
    if ((value = keybound(n, 0)) != 0) {
      size_t need = strlen(value) + 2;
      char *temp = malloc(sizeof(char) * need);
      define_key(temp, n + MY_KEYS);
      free(temp);
      free(value);
    }
  }

  while ((ch = getch()) != ERR) {
    bool escaped = (ch >= MY_KEYS);
    const char *name = keyname(escaped ? (ch - MY_KEYS) : ch);
    printw("Keycode %d, name %s%s\n",
        ch,
        escaped ? "ESC-" : "",
        name != 0 ? name : "<null>");
    clrtoeol();
    if (ch == 'q')
      break;
  }
  endwin();
  return;
}

