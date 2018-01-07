#pragma once

#include <string>

struct imsgbuf;

int do_connect(std::string const& address, const short port);
int do_read(imsgbuf *ibuf, const int sockfd);
