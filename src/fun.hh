#pragma once

#include <poll.h>

struct imsgbuf;

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
  bool refresh;
  char content[128];
};

extern "C" {
  int do_read(struct imsgbuf *ibuf, const int sockfd);
  int do_user(struct imsgbuf *display_ibuf);
  int do_display(struct imsgbuf *net_ibuf, struct imsgbuf *user_ibuf);
}

template <typename Function>
void
network_read(pollfd *pfd, const unsigned nfd, Function fun) {
  while (true) {
    int nready = poll(pfd, nfd, 60 * 1000);
    if (nready == -1) {
      err(1, "poll");
      exit(42);
    }
    if (nready == 0)
      continue;
    for (unsigned i = 0; i < nfd; ++i) {
      if (pfd[i].revents & (POLLERR|POLLNVAL)) {
	err(1, "bad fd %d", pfd[i].fd);
        exit(42);
      }

      if (pfd[i].revents & (POLLIN|POLLHUP)) {
        fun(i);
      }
    }
  }
}
