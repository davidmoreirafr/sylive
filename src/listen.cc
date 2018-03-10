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
__BEGIN_DECLS
#include <imsg.h>
__END_DECLS

#include <vector>

#include <fun.hh>
#include <utils.hh>

#define SMALL_READ_BUF 1024

/*
 * Method to read each line from the socket
 */

int do_read(imsgbuf *ibuf, const int sockfd) {
  char buf[SMALL_READ_BUF];
  std::vector<std::string> current_lines;
  current_lines.resize(1);

  struct pollfd pfd[1]; // FIXME: Manage add/remove fd
  int nfd = 1;
  pfd[0].fd = sockfd;
  pfd[0].events = POLLIN;

  network_read(pfd, nfd, [&](unsigned i) {
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
    });
  return -1;
}
