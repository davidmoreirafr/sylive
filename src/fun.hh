#pragma once

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
  char content[1024];
};

void tell_user(int line, Placement placement, std::string const& value, imsgbuf *user_ibuf);
int do_connect(std::string const& address, const short port);
int do_read(imsgbuf *ibuf, const int sockfd);
int do_user(imsgbuf *display_ibuf);
int do_display(imsgbuf *net_ibuf, imsgbuf *user_ibuf);
