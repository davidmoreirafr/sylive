#include <sys/types.h>
#include <sys/queue.h>
#include <sys/uio.h>
#include <stdint.h>
#include <imsg.h>
#include <err.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>


int do_read(struct imsgbuf *ibuf, const int sockfd);
int do_user(struct imsgbuf *display_ibuf);
int do_display(struct imsgbuf *net_ibuf, struct imsgbuf *user_ibuf);

int do_connect(char * address, const short port) {
  struct sockaddr_in serv_addr;
 
  int sockfd;
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    err(1, "listen");
 
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);
  serv_addr.sin_addr.s_addr = inet_addr(address);
 
  if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    err(1, "listen");

  return sockfd;
}

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
    struct imsgbuf display_ibuf;
    imsg_init(&display_ibuf, user_imsg_fds[1]);
    exit(do_user(&display_ibuf)); // Ahah
  }
}
void fork_display(int imsg_fds[2], int connection_socket) {
  switch (fork()) {
  case -1:
    err(1, "fork");
  case 0:
    close(connection_socket);
    if (pledge("stdio proc rpath tty", NULL))
      err(2, "pledge");

    close(imsg_fds[0]);
    struct imsgbuf network_ibuf;
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
    struct imsgbuf user_ibuf;
    imsg_init(&user_ibuf, user_imsg_fds[0]);

    exit(do_display(&network_ibuf, &user_ibuf));
  }
}
int main(int argc, char *argv[]) {
  if (pledge("stdio inet proc rpath tty", NULL))
    err(2, "pledge");

  if (argc == 3) {
    const char *errstr = 0;
    int imsg_fds[2];
    short port = strtonum(argv[2], 1, 65536, &errstr);
    const int sockfd = do_connect(argv[1], port);
    if (errstr != NULL)
      errx(1, "port number is %s: %s", errstr, argv[1]);

    if (socketpair(AF_UNIX, SOCK_STREAM, PF_UNSPEC, imsg_fds) == -1)
      err(2, "socketpair");

    fork_display(imsg_fds, sockfd);

    close(imsg_fds[1]);
    close_std();
    struct imsgbuf display_ibuf;
    imsg_init(&display_ibuf, imsg_fds[0]);
    {
      if (pledge("stdio inet", NULL))
      	err(2, "pledge");
      exit(do_read(&display_ibuf, sockfd));
    }
  }
  return 2;
}
