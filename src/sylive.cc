#include <err.h>

#include <iomanip>
#include <iostream>

#include <boost/lexical_cast.hpp>

#include <listen.hh>

int main(int argc, char *argv[])
{
  if (pledge("stdio inet", NULL))
    err(2, "pledge");
  std::cout << argc << std::endl;
  if (argc == 3) {
    std::cout << std::fixed << std::setprecision(1);
    const int sockfd = do_connect(argv[1], boost::lexical_cast<short>(argv[2]));
    if (pledge("stdio", NULL))
      err(2, "pledge");
    do_read(sockfd);
  }
}
