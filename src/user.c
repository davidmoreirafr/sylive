#include <string.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <sys/uio.h>
#include <stdint.h>
#include <imsg.h>
#include <err.h>
#include <stdlib.h>
#include <errno.h>

#include <curses.h>

#include <utils.h>

static struct Line *lines = NULL;
int line_number = 0;

void
update_keys(struct imsgbuf *display_buf) {
  while (true) {
    int key = getch();

    if (key == ERR)
      break;

    compose(display_buf, IMSG_KEY, &key, sizeof key);
  }
}

void
display_screen() {
  for (int i = 0; i < line_number; ++i) {
    if (lines[i].line != 0 && lines[i].refresh) {
      lines[i].refresh = false;
      int col;
      switch (lines[i].placement) {
      case LEFT:
	col = 0;
	break;
      case CENTER:
	col = (COLS - strlen(lines[i].content)) / 2;
	break;
      case RIGHT:
	col = COLS - strlen(lines[i].content) + 1;
      }
      if (col < 0)
	col = 0;
      if (col > COLS)
	col = COLS;
      mvprintw(i, col, lines[i].content);
      clrtoeol();
    }
  }
  move(0, 0);
  refresh();
}

int
do_user(struct imsgbuf *display_ibuf) {
  // init screen
  initscr();
  nodelay(stdscr, TRUE);
  noecho();

  for (;;) {
    update_keys(display_ibuf);
    display_screen();
    int n = imsg_read(display_ibuf);
    if (n == -1) {
      endwin();
      err(1, "imsg_get");
    }
    if (n == 0) {
      endwin();
      return 0;
    }

    // Read the messages
    for (;;) {
      struct imsg imsg;
      n = imsg_get(display_ibuf, &imsg);
      if (n == -1) {
	endwin();
	err(1, "imsg_get");
      }
      if (n == 0)
	break;

      switch (imsg.hdr.type) {
      case IMSG_LINE: {
	if (line_number != LINES) {
	  free(lines);
	  lines = (struct Line *) calloc(LINES, sizeof (struct Line));
	  memset(lines, 0, sizeof(struct Line) * LINES);
	  line_number = LINES;
	}
	struct Line *l = (struct Line *)imsg.data;	
	if (line_number > l->line) {
	  lines[l->line] = *l;
	}
	break;
      }
      default:
	endwin();
	err(1, "bad message type %d", imsg.hdr.type);
	break;
      }

      imsg_free(&imsg);
    }
  }
}
