#pragma once

#include <sys/types.h>
#include <sys/queue.h>
#include <sys/uio.h>
#include <stdint.h>
__BEGIN_DECLS
#include <imsg.h>
__END_DECLS

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
  enum Placement placement;
  char refresh;
  char content[128];
};

#ifdef __cplusplsuy
extern "C" {
#endif
  int do_read(struct imsgbuf *ibuf, const int sockfd);
  int do_user(struct imsgbuf *display_ibuf);
  int do_display(struct imsgbuf *net_ibuf, struct imsgbuf *user_ibuf);

  double membyte(unsigned long long ll);
  char membyte_unit(unsigned long long ll);
  double byte(unsigned long long ll);
  char byte_unit(unsigned long long ll);
  void compose(struct imsgbuf *ibuf, int type, void const * data, int len);
#ifdef __cplusplus
}
#endif
