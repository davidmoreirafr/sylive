#include <curses.h>

#include <display.hh>

void
tell_display(int line, Placement placement, std::string const& value) {
  int col;
  switch (placement) {
  case LEFT:
    col = 0;
    break;
  case CENTER:
    col = (COLS - value.length()) / 2;
    break;
  case RIGHT:
    col = COLS - value.length() + 1;
    break;
  }
  if (col < 0)
    col = 0;
  if (col > COLS)
    col = COLS;

  mvprintw(line, col, value.c_str());
}
