#include <listen.hh>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <err.h>
#include <sys/queue.h>
#include <imsg.h>
#include <poll.h>

#include <iostream>
#include <algorithm>
#include <iterator>
#include <map>
#include <list>
#include <iomanip>

#include <boost/lexical_cast.hpp>

#include <type.hh>
#include <utils.hh>

#define SMALL_READ_BUF 1024

/*
 * Method to read each line from the socket
 */

int do_connect(std::string const& address, const short port) {
  struct sockaddr_in serv_addr;
 
  int sockfd;
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    err(1, "listen");
 
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);
  serv_addr.sin_addr.s_addr = inet_addr(address.c_str());
 
  if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    err(1, "listen");

  return sockfd;
}
int do_read(imsgbuf *ibuf, const int sockfd) {
  char buf[SMALL_READ_BUF];
  std::vector<std::string> current_lines;
  current_lines.resize(1);

  struct pollfd pfd[16]; // FIXME: Manage add/remove fd
  int nfd = 1;
  int nready;

  pfd[0].fd = sockfd;
  pfd[0].events = POLLIN;
  while (true) {
    nready = poll(pfd, nfd, 60 * 1000);
    if (nready == -1)
      err(1, "poll");
    if (nready == 0)
      continue;
    for (int i = 0; i < nfd; ++i) {
      if ((pfd[i].revents & (POLLERR|POLLNVAL)))
	errx(1, "bad fd %d", pfd[i].fd);
      if (pfd[i].revents & (POLLIN|POLLHUP)) {
	int nb_read = read(pfd[i].fd, buf, sizeof(buf) -1);
	buf[nb_read] = 0;
	std::string chunk(buf);
	while (!chunk.empty()) {
	  auto find = chunk.find("\n");
	  if (find == std::string::npos) {
	    current_lines[i] += chunk;
	    break;
	  }
	  else {
	    current_lines[0] += chunk.substr(0, find);
	    chunk = chunk.substr(find + 1);
	    compose(ibuf, IMSG_DATA, current_lines[0].c_str(), current_lines[0].length() + 1);
	    current_lines[0] = "";
	  }
	}
      }
    }
  }
  return -1;
}
