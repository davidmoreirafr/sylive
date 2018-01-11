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

// FIXME
int do_read(struct imsgbuf *ibuf, const int sockfd);
int do_user(struct imsgbuf *display_ibuf);
int do_display(struct imsgbuf *net_ibuf, struct imsgbuf *user_ibuf);
//#include <utils.hh>
static
void compose(struct imsgbuf *ibuf, int type, void const * data, int len) {
  imsg_compose(ibuf, type, 0, 0, -1, data, len);
  while (true) {
    int write = msgbuf_write(&ibuf->w);
    if (write == -1 && errno != EAGAIN)
      err(1, "msgbuf_write");
    if (write == 0)
      break;
  }
}

enum imsg_type {
  IMSG_DATA,
  IMSG_LINE,
  IMSG_KEY
};

enum Placement {
  LEFT,
  CENTER,
  RIGHT
};

struct Line {
  int line;
  enum Placement placement;
  char content[128];
};

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
  int i; // FIXME
  clear();
  for (i = 0; i < line_number; ++i) {
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
do_user(struct imsgbuf *display_ibuf) {
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
    update_keys(display_ibuf);
    display_screen();
  }
}
