#pragma once


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
  Placement placement;
  char content[1024];
};

