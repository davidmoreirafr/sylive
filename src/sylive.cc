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
#include <view.hh>

void
close_std() {
  close(0);
  close(1);
  close(2);
}

void
fork_display(int imsg_fds[2]) {
  switch (fork()) {
  case -1:
    err(1, "fork");
  case 0:
    if (pledge("stdio rpath tty", NULL))
      err(2, "pledge");

    close(imsg_fds[0]);
    imsgbuf network_ibuf;
    imsg_init(&network_ibuf, imsg_fds[1]);

    // FIXME: create user_ibuf
    std::exit(do_display(&network_ibuf)); // FIXME: user_ibuf));
  }
}

int main(int argc, char *argv[])
{
  if (pledge("stdio inet proc rpath tty", NULL))
    err(2, "pledge");

  if (argc == 3) {
    int imsg_fds[2];

    if (socketpair(AF_UNIX, SOCK_STREAM, PF_UNSPEC, imsg_fds) == -1)
      err(2, "socketpair");

    fork_display(imsg_fds);

    close(imsg_fds[1]);
    close_std();
    imsgbuf display_ibuf;
    imsg_init(&display_ibuf, imsg_fds[0]);
    {
      const int sockfd = do_connect(argv[1], boost::lexical_cast<short>(argv[2]));
      if (pledge("stdio inet", NULL))
      	err(2, "pledge");
      std::exit(do_read(&display_ibuf, sockfd));
    }
  }
}
