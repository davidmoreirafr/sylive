#pragma once

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
double membyte(unsigned long long ll) {
  double d = ll;
  do {
    d /= 1024;
  } while (d > 9999);
  return d;
}

inline
char membyte_unit(unsigned long long ll) {
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
double byte(unsigned long long ll) {
  double d = ll;
  while (d > 9999) {
    d /= 1024;
  }
  return d;
}

inline
char byte_unit(unsigned long long ll) {
  const static char *unit = " kmgtep";
  int ct = 0;
  double d = ll;
  while (d > 9999) {
    d /= 1024;
    ++ct;
  }
  return unit[ct];
}

// FIXME: Should be C only
inline
void compose(imsgbuf *ibuf, int type, void const * data, int len) {
  imsg_compose(ibuf, type, 0, 0, -1, data, len);
  while (true) {
    int write = msgbuf_write(&ibuf->w);
    if (write == -1 && errno != EAGAIN)
      err(1, "msgbuf_write");
    if (write == 0)
      break;
  }
}
