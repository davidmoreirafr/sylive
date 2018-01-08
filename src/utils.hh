#pragma once

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
