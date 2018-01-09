#pragma once

#include <string>

enum Placement {
  LEFT,
  CENTER,
  RIGHT
};

void
tell_display(int line, Placement placement, std::string const& value);
