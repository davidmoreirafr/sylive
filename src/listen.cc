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

#include <iostream>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <map>
#include <list>
#include <iomanip>

#include <boost/lexical_cast.hpp>

#include <type.hh>

#define SMALL_READ_BUF 1024

/*
 * Method to read each line from the socket
 */

int
do_connect(std::string const& address, const short port) {
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

int
do_read(imsgbuf *ibuf, const int sockfd) {
  char buf[SMALL_READ_BUF];
  std::string current_line;

  int nb_read;
  while ((nb_read = read(sockfd, buf, sizeof(buf) -1)) > 0) {
    buf[nb_read] = 0;
    std::string chunk(buf);
    while (!chunk.empty()) {
      auto find = chunk.find("\n");

      if (find == std::string::npos) {
	current_line += chunk;
	break;
      }
      else {
	current_line += chunk.substr(0, find);
	chunk = chunk.substr(find + 1);
	imsg_compose(ibuf, IMSG_DATA,
		     0, 0, -1,
		     current_line.c_str(),
		     current_line.length() + 1);
	while (true) {
	  int write = msgbuf_write(&ibuf->w);
	  if (write == -1 && errno != EAGAIN)
	    err(1, "msgbuf_write");
	  if (write == 0)
	    break;
	}
	current_line = "";
      }
    }
  }
  return -1;
}
