#include <string.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <sys/uio.h>
#include <stdint.h>
#include <imsg.h>
#include <err.h>

#include <map>
#include <iostream>

#include <curses.h>

#include <type.hh>
#include <user.hh>
#include <utils.hh>

void
update_keys(imsgbuf *display_buf) {
  while (true) {
    int key = getch();

    if (key == ERR)
      break;

    compose(display_buf, IMSG_KEY, &key, sizeof key);

    // send key to display proc
  }
}

void
display_screen(std::map<int, Line> const& lines) {
  clear();
  for(std::pair<int, Line> line: lines) {
    if (line.first < LINES) {
      int col;
      switch (line.second.placement) {
      case LEFT:
	col = 0;
	break;
      case CENTER:
	col = (COLS - strlen(line.second.content)) / 2;
	break;
      case RIGHT:
	col = COLS - strlen(line.second.content) + 1;
      }
      if (col < 0)
	col = 0;
      if (col > COLS)
	col = COLS;

      mvprintw(line.first, col, line.second.content);
    }
  }
  refresh();
}

int
do_user(imsgbuf *display_ibuf) {
  std::map<int, Line> lines;
  // // init screen
  initscr();
  nodelay(stdscr, TRUE);
  noecho();

  while (true) {
    for (;;) {
      int n = imsg_read(display_ibuf);
      if (n == -1) {
	endwin();
	err(1, "imsg_get");
      }
      if (n == 0) {
	endwin();
	return 0; // Socket closed
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
	  // FIXME: Check corrupt data
	  Line *l = (Line *)imsg.data;
	  lines[l->line] = *l;
	  break;
	}
	default:
	  endwin();
	  err(1, "bad message type %d", imsg.hdr.type);
	  break;
	}

	imsg_free(&imsg);
      }
      display_screen(lines);
    }
    clear();
    mvprintw(0, 0, "FIXME!");
    refresh();
  }
}
