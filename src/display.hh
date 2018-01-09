#pragma once

#include <string>

#include <type.hh>

struct imsgbuf;

void
tell_user(int line, Placement placement, std::string const& value, imsgbuf *user_ibuf);
