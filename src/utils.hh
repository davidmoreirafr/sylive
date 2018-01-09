#pragma once
#include <string>
#include <sstream>

template<typename Out>
void split(const std::string &s, char delim, Out result) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}

inline
void
byte(char l[1024], long long ll) {
  const static char *unit = "\0kmgtep";
  int ct = 0;
  double d = ll;
  while (d > 999) {
    d /= 1024;
    ++ct;
  }

  char c[16];
  snprintf(c, 15, "%.2f%c", d, unit[ct]);
  strncat(l, c, 1023);
}

inline
double
membyte(unsigned long long ll) {
  double d = ll;
  do {
    d /= 1024;
  } while (d > 9999);
  return d;
}

inline
char
membyte_unit(unsigned long long ll) {
  const static char *unit = " kmgtep";
  int ct = 0;
  double d = ll;
  do {
    d /= 1024;
    ++ct;
  } while (d > 9999);
  return unit[ct];
}

inline
double
byte(unsigned long long ll) {
  double d = ll;
  while (d > 9999) {
    d /= 1024;
  }
  return d;
}

inline
char
byte_unit(unsigned long long ll) {
  const static char *unit = " kmgtep";
  int ct = 0;
  double d = ll;
  while (d > 9999) {
    d /= 1024;
    ++ct;
  }
  return unit[ct];
}

inline
void
compose(imsgbuf *ibuf, int type, void *data, int len) {
  imsg_compose(ibuf, type, 0, 0, -1, data, len);
  while (true) {
    int write = msgbuf_write(&ibuf->w);
    if (write == -1 && errno != EAGAIN)
      err(1, "msgbuf_write");
    if (write == 0)
      break;
  }
}
