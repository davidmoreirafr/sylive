#include <sys/types.h>
#include <sys/queue.h>
#include <sys/uio.h>
#include <stdint.h>
#include <imsg.h>
#include <err.h>
#include <sys/socket.h>

#include <iomanip>
#include <iostream>

#include <boost/lexical_cast.hpp>

#include <listen.hh>

int do_display(imsgbuf *); // FIXME

int main(int argc, char *argv[])
{
  if (pledge("stdio inet proc", NULL))
    err(2, "pledge");
  if (argc == 3) {
    int imsg_fds[2];

    if (socketpair(AF_UNIX, SOCK_STREAM, PF_UNSPEC, imsg_fds) == -1)
      err(2, "socketpair");
    switch (fork()) {
    case -1:
      err(1, "fork");
    case 0:
      /*child*/
      if (pledge("stdio", NULL))
      	err(2, "pledge");
      close(imsg_fds[0]);
      imsgbuf child_ibuf;
      imsg_init(&child_ibuf, imsg_fds[1]);
      std::cout << std::fixed << std::setprecision(1);
      std::exit(do_display(&child_ibuf));
    }

    /*parent*/
    close(imsg_fds[1]);
    close(0);
    close(1);
    close(2);
    imsgbuf parent_ibuf;
    imsg_init(&parent_ibuf, imsg_fds[0]);

    {
      const int sockfd = do_connect(argv[1], boost::lexical_cast<short>(argv[2]));
      if (pledge("stdio", NULL))
      	err(2, "pledge");
      exit(do_read(&parent_ibuf, sockfd));
    }
  }
}
