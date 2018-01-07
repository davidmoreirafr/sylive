#include <listen.hh>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <err.h>

#include <iostream>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <map>
#include <list>
#include <iomanip>

#include <boost/lexical_cast.hpp>

#include <utils.hh>
#define SMALL_READ_BUF 1024

using host_t = std::string;

struct cpu_t {
  long long date;

  float user;
  float nice;
  float system;
  float interrupt;
  float idle;
};

struct if_t {
  unsigned long long ipackets;
  unsigned long long opackets;
  unsigned long long ibytes;
  unsigned long long obytes;
  unsigned long long imcasts;
  unsigned long long omcasts;
  unsigned long long ierrors;
  unsigned long long oerrors;
  unsigned long long collisions;
  unsigned long long drops;
};

struct proc_t {
  long long uticks;
  long long sticks;
  long long iticks;
  long long cpusec;
  long long cpupct;
  long long procsz;
  long long rsssz;
};

struct datum_t {
  std::vector<cpu_t> cpus;
  std::map<std::string, double> sensors;
  std::map<std::string, if_t> nics;
  std::map<std::string, proc_t> procs;
};

void
display_cpu(std::ostream &ost,
	    int nb,
	    double user, double nice, double system, double interrupt, double idle) {
  if (nb >= 0)
    ost << "cpu" << nb << ": ";
  else
    ost << "cpu: ";
  ost << user << "% user, "
      << nice << "% nice, "
      << system << "% system, "
      << interrupt << "% interrupt, "
      << idle << "% idle" << std::endl;
}

void
cpu_view(std::ostream &ost,
	 std::string const& hostname,
	 std::vector<cpu_t> const& cpus,
	 bool extended_view) {
  double user, nice ,system, interrupt, idle;
  user = nice = system = interrupt = idle = 0;

  int cpu_number = 0;
  for (cpu_t cpu: cpus) {
    if (extended_view)
      ost << hostname
	  << "\t" << cpu_number
	  << "\t" << cpu.user
	  << "\t" << cpu.nice
	  << "\t" << cpu.system
	  << "\t" << cpu.interrupt
	  << "\t\t" << cpu.idle
	  << std::endl;
    user += cpu.user;
    nice += cpu.nice;
    system += cpu.system;
    interrupt += cpu.interrupt;
    idle += cpu.idle;
    ++cpu_number;
  }
  ost << hostname
      << "\t" << " "
      << "\t" << user / cpu_number
      << "\t" << nice / cpu_number
      << "\t" << system / cpu_number
      << "\t" << interrupt / cpu_number
      << "\t\t" << idle / cpu_number
      << std::endl;
}

void
byte(std::ostream &ost, long long ll) {
  const static std::string unit = " kmgtep";
  int ct = 0;
  double d = ll;
  while (d > 999) {
    d /= 1024;
    ++ct;
  }
  ost << std::setprecision(2)
      << d
      << std::setprecision(1);
  if (ct)
    ost << unit[ct];
}

void
proc_view(std::ostream &ost, std::string const& hostname, std::map<std::string, proc_t> procs) {
  for (std::pair<std::string, proc_t> proc: procs) {
    ost << proc.first
	<< "\t" << hostname
	<< "\t" << proc.second.uticks
	<< "\t" << proc.second.sticks
	<< "\t" << proc.second.iticks
	<< "\t" << proc.second.cpusec
	<< "\t" << proc.second.cpupct
	<< "\t";
    byte(ost, proc.second.procsz);
    ost << "\t";
    byte(ost, proc.second.rsssz);
    ost << std::endl;
  }
}

void
print(std::map<std::string, datum_t> data) {
  std::cout << "host" << "\t"
	    << "#" << "\t"
	    << "User" << "\t"
	    << "Nice" << "\t"
	    <<"System" << "\t"
	    << "Interrupt" << "\t"
	    <<"Idle" << std::endl;
  for (std::pair<std::string, datum_t> datum: data)
    cpu_view(std::cout, datum.first, datum.second.cpus, false);
  std::cout << std::endl;

  std::cout
    << "proc"
    << "\t" << "host" << "\t"
    << "\t" << "uticks"
    << "\t" << "sticks"
    << "\t" << "iticks"
    << "\t" << "cpusec"
    << "\t" << "cpupct"
    << "\t" << "procsz"
    << "\t" << "rsssz"
    << std::endl;

  for (std::pair<std::string, datum_t> datum: data)
    proc_view(std::cout, datum.first, datum.second.procs);
  std::cout << std::endl;

}

double
todouble(std::string const& str) {
  return boost::lexical_cast<double>(str);
}

void
treat_line(std::string const& line) {
  static std::map<host_t, datum_t> data;
  datum_t datum;
  datum.cpus.resize(4); // FIXME

  std::list<std::string> splitted_line;
  split(line, ';', std::back_inserter(splitted_line));
  if (splitted_line.empty())
    return;

  std::string host = splitted_line.front();
  //std::cout << "receiving datum for: " << splitted_line.front() << std::endl;
  splitted_line.pop_front();

  for (std::string measure_raw: splitted_line) {
    std::vector<std::string> measure;
    split(measure_raw, ':', std::back_inserter(measure));
    if (measure.empty())
      continue;

    if (measure[0] == "cpu") {
      int cpu = boost::lexical_cast<int>(measure[1]);
      datum.cpus[cpu].date
	= boost::lexical_cast<long long>(measure[2]);

      datum.cpus[cpu].user = todouble(measure[3]);
      datum.cpus[cpu].nice = todouble(measure[4]);
      datum.cpus[cpu].system = todouble(measure[5]);
      datum.cpus[cpu].interrupt = todouble(measure[5]);
      datum.cpus[cpu].idle = todouble(measure[7]);
    }
    else if (measure[0] == "if") {
      const std::string nic = measure[1];
      datum.nics[nic].ipackets = todouble(measure[2]);
      datum.nics[nic].opackets = todouble(measure[3]);
      datum.nics[nic].ibytes = todouble(measure[4]);
      datum.nics[nic].obytes = todouble(measure[5]);
      datum.nics[nic].imcasts = todouble(measure[6]);
      datum.nics[nic].omcasts = todouble(measure[7]);
      datum.nics[nic].ierrors = todouble(measure[8]);
      datum.nics[nic].oerrors = todouble(measure[9]);
      datum.nics[nic].collisions = todouble(measure[10]);
      datum.nics[nic].drops = todouble(measure[11]);

    }
    else if (measure[0] == "sensor") {
      datum.sensors[measure[1]] = todouble(measure[3]);
    }
    else if (measure[0] == "proc") {
      datum.procs[measure[1]].uticks = todouble(measure[4]);
      datum.procs[measure[1]].sticks = todouble(measure[5]);
      datum.procs[measure[1]].iticks = todouble(measure[6]);
      datum.procs[measure[1]].cpusec = todouble(measure[7]);
      datum.procs[measure[1]].cpupct = todouble(measure[8]);
      datum.procs[measure[1]].procsz = todouble(measure[9]);
      datum.procs[measure[1]].rsssz = todouble(measure[10]);      
    }
    else {
      std::cerr << "unknown measure: " << measure_raw << std::endl;
    }
  }
  
  data[host] = datum;
  print(data);
}

/*
 * Method to read each line from the socket
 */

int
do_connect(std::string const& address, const short port) {
  struct sockaddr_in serv_addr;
 
  int sockfd;
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    err(1, "listen");
 
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);
  serv_addr.sin_addr.s_addr = inet_addr(address.c_str());
 
  if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    err(1, "listen");

  return sockfd;
}

int
do_read(imsgbuf *, const int sockfd) {
  char buf[SMALL_READ_BUF];
  std::string current_line;

  int nb_read;
  while ((nb_read = read(sockfd, buf, sizeof(buf) -1)) > 0) {
    buf[nb_read] = 0;
    std::string chunk(buf);
    while (!chunk.empty()) {
      auto find = chunk.find("\n");

      if (find == std::string::npos) {
	current_line += chunk;
	break;
      }
      else {
	current_line += chunk.substr(0, find);
	chunk = chunk.substr(find + 1);
	treat_line(current_line);
	current_line = "";
      }
    }
  }
  return -1;
}
