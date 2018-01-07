#pragma once

#include <string>

int do_connect(std::string const& address, const short port);
void do_read(const int sockfd);
