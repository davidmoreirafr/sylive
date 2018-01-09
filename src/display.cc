#include <string.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <sys/uio.h>
#include <stdint.h>
#include <imsg.h>
#include <err.h>

#include <curses.h>

#include <display.hh>
#include <type.hh>

void
tell_user(int line, Placement placement, std::string const& value, imsgbuf *user_ibuf) {
  Line l;
  l.line = line;
  l.placement = placement;
  strncpy(l.content, value.c_str(), 1024);
  imsg_compose(user_ibuf, IMSG_LINE, 0, 0, -1, &l, sizeof l);
  while (true) {
    int write = msgbuf_write(&user_ibuf->w);
    if (write == -1 && errno != EAGAIN)
      err(1, "msgbuf_write");
    if (write == 0)
      break;
  }
}
