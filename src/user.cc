#include <string.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <sys/uio.h>
#include <stdint.h>
#include <imsg.h>
#include <err.h>

#include <curses.h>

#include <fun.hh>
#include <utils.hh>

static Line *lines = NULL;
int line_number = 0;

void
update_keys(imsgbuf *display_buf) {
  while (true) {
    int key = getch();

    if (key == ERR)
      break;

    compose(display_buf, IMSG_KEY, &key, sizeof key);
  }
}

void
display_screen() {
  clear();
  for (int i = 0; i < line_number; ++i) {
    if (lines[i].line != 0) {
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
    }
  }
  move(0, 0);
  refresh();
}


int
do_user(imsgbuf *display_ibuf) {
  // init screen
  initscr();
  nodelay(stdscr, TRUE);
  noecho();

  for (;;) {
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
      imsg imsg;
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
	  lines = (Line *) calloc(LINES, sizeof (Line));
	  memset(lines, 0, sizeof(Line) * LINES);
	  line_number = LINES;
	}
	  Line *l = (Line *)imsg.data;	
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
    update_keys(display_ibuf);
    display_screen();
  }
}
