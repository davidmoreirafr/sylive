#include <errno.h>
#include <err.h>

#include <utils.h>

double membyte(unsigned long long ll) {
  double d = ll;
  do {
    d /= 1024;
  } while (d > 9999);
  return d;
}

char membyte_unit(unsigned long long ll) {
  static const char *unit = " kmgtep";
  int ct = 0;
  double d = ll;
  do {
    d /= 1024;
    ++ct;
  } while (d > 9999);
  return unit[ct];
}

double byte(unsigned long long ll) {
  double d = ll;
  while (d > 9999) {
    d /= 1024;
  }
  return d;
}

char byte_unit(unsigned long long ll) {
  static const char *unit = " kmgtep";
  int ct = 0;
  double d = ll;
  while (d > 9999) {
    d /= 1024;
    ++ct;
  }
  return unit[ct];
}

void compose(struct imsgbuf *ibuf, int type, void const * data, int len) {
  imsg_compose(ibuf, type, 0, 0, -1, data, len);
  while (1) {
    int write = msgbuf_write(&ibuf->w);
    if (write == -1 && errno != EAGAIN)
      err(1, "msgbuf_write");
    if (write == 0)
      break;
  }
}

