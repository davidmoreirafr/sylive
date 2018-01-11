#pragma once

#include <poll.h>

#include <string>

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
  Placement placement;
  char content[256];
};

void tell_user(int line, Placement placement, std::string const& value, imsgbuf *user_ibuf);
int do_connect(std::string const& address, const short port);
int do_read(imsgbuf *ibuf, const int sockfd);
int do_user(imsgbuf *display_ibuf);
int do_display(imsgbuf *net_ibuf, imsgbuf *user_ibuf);

template <typename Function>
void
network_read(pollfd *pfd, const unsigned nfd, Function fun) {
  while (true) {
    int nready = poll(pfd, nfd, 60 * 1000);
    if (nready == -1)
      err(1, "poll");
    if (nready == 0)
      continue;
    for (unsigned i = 0; i < nfd; ++i) {
      if (pfd[i].revents & (POLLERR|POLLNVAL))
	err(1, "bad fd %d", pfd[i].fd);
      if (pfd[i].revents & (POLLIN|POLLHUP)) {
	fun(i);
      }
    }
  }
}
