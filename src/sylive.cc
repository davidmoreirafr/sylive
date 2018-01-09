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
#include <user.hh>

void close_std() {
  close(0);
  close(1);
  close(2);
}
void fuck_user(int user_imsg_fds[2]) {
  switch (fork()) {
  case -1:
    err(1, "fork");
  case 0:
    if (pledge("stdio tty rpath", NULL))
      err(2, "pledge");

    close(user_imsg_fds[0]);
    imsgbuf display_ibuf;
    imsg_init(&display_ibuf, user_imsg_fds[1]);
    std::exit(do_user(&display_ibuf)); // Ahah
  }
}
void fork_display(int imsg_fds[2]) {
  switch (fork()) {
  case -1:
    err(1, "fork");
  case 0:
    if (pledge("stdio proc rpath tty", NULL))
      err(2, "pledge");

    close(imsg_fds[0]);
    imsgbuf network_ibuf;
    imsg_init(&network_ibuf, imsg_fds[1]);

    // Create the user processus
    int user_imsg_fds[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, PF_UNSPEC, user_imsg_fds) == -1)
      err(2, "socketpair");

    fuck_user(user_imsg_fds);
    if (pledge("stdio", NULL))
      err(2, "pledge");
    close_std();

    close(user_imsg_fds[1]);
    close_std();
    imsgbuf user_ibuf;
    imsg_init(&user_ibuf, user_imsg_fds[0]);

    std::exit(do_display(&network_ibuf, &user_ibuf));
  }
}
int main(int argc, char *argv[]) {
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
