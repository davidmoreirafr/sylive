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
#include <sys/queue.h>
#include <imsg.h>

#include <curses.h>

#include <iostream>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <map>
#include <list>
#include <iomanip>

#include <boost/lexical_cast.hpp>

#include <display.hh>
#include <type.hh>
#include <utils.hh>

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

struct mem_t {
  unsigned long long real_active;
  unsigned long long real_total;
  unsigned long long free;
  unsigned long long swap_used;
  unsigned long long swap_total;
};

struct datum_t {
  std::vector<cpu_t> cpus;
  std::map<std::string, double> sensors;
  std::map<std::string, if_t> nics;
  std::map<std::string, proc_t> procs;
  mem_t mem;
};


struct param_t {
  bool detailed_cpu;
  bool show_mem;
  bool show_procs;
  bool show_nics;
  unsigned width, height;
};

static param_t param;

void
cpu_view(unsigned &line,
	 std::string const& hostname,
	 std::vector<cpu_t> const& cpus) {
  double user, nice ,system, interrupt, idle;
  char l[1024];
  user = nice = system = interrupt = idle = 0;

  int cpu_number = 0;
  bool first = true;
  for (cpu_t cpu: cpus) {
    if (param.detailed_cpu) {
      snprintf(l, 1024,"%s\t%d\t%.2f\t%.2f\t%.2f\t%.2f\t\t%.2f",
      	       first ? hostname.c_str() : "\t",
      	       cpu_number,
      	       cpu.user,
      	       cpu.nice,
      	       cpu.system,
      	       cpu.interrupt,
      	       cpu.idle);
      tell_display(line++, LEFT, l);
      first = false;
    }
    user += cpu.user;
    nice += cpu.nice;
    system += cpu.system;
    interrupt += cpu.interrupt;
    idle += cpu.idle;
    ++cpu_number;

  }
  snprintf(l, 1024, "%s\t \t%.2f\t%.2f\t%.2f\t%.2f\t\t%.2f",
  	   first ? hostname.c_str() : "\t",
  	   user / cpu_number,
  	   nice / cpu_number,
  	   system / cpu_number,
  	   interrupt / cpu_number,
  	   idle / cpu_number);
  tell_display(line++, LEFT, l);
}

void
proc_view(unsigned &line, std::string const& hostname, std::map<std::string, proc_t> procs) {
  char l[1024];
  for (std::pair<std::string, proc_t> proc: procs) {
    snprintf(l, 1023,
	     "%s\t%llu\t%llu\t%llu\t%llu\t%llu\t",
	     hostname.c_str(),
	     proc.second.uticks,
	     proc.second.sticks,
	     proc.second.iticks,
	     proc.second.cpusec,
	     proc.second.cpupct);
    byte(l, proc.second.procsz);
    strlcat(l, "\t", 1023);
    byte(l, proc.second.rsssz);
    strlcat(l, "\t", 1023);
    strlcat(l, proc.first.c_str(), 1023);

    tell_display(line++, LEFT, l);
  }
}

void
display_date(unsigned &line) {
  time_t tod = time(NULL);
  char *ctod = ctime(&tod);
  char l[64];
  snprintf(l, 64, "%s", ctod);
  tell_display(line++, RIGHT, l);
}

void
mem_view(unsigned &line, std::string const& hostname, mem_t const& mem) {
  char l[1024];
  snprintf(l, 1024, "%s\t%.0f%c/%.0f%c\t%.0f%c\t%.0f%c/%.0f%c",
	   hostname.c_str(),
	   membyte(mem.real_active),
	   membyte_unit(mem.real_active),
	   membyte(mem.real_total),
	   membyte_unit(mem.real_total),
	   membyte(mem.free),
	   membyte_unit(mem.free),
	   membyte(mem.swap_used),
	   membyte_unit(mem.swap_used),
	   membyte(mem.swap_total),
	   membyte_unit(mem.swap_total)
	   );
  tell_display(line++, LEFT, l);
}

void
show_nics(unsigned &line, std::string const& hostname, std::map<std::string, if_t> const& nics) {
  char l[1024];
  bool first = true;
  for (std::pair<std::string, if_t> nic: nics) {
    snprintf(l, 1024, "%s\t%s\t%.0f%c\t%.0f%c\t%.0f%c\t%.0f%c\t%.0f%c\t%.0f%c\t%.0f%c\t%.0f%c",
	     first ? hostname.c_str() : "\t",
	     nic.first.c_str(),
	     membyte(nic.second.ipackets),
	     membyte_unit(nic.second.ipackets),
	     membyte(nic.second.opackets),
	     membyte_unit(nic.second.opackets),
	     membyte(nic.second.ibytes),
	     membyte_unit(nic.second.ibytes),
	     membyte(nic.second.obytes),
	     membyte_unit(nic.second.obytes),
	     byte(nic.second.ierrors),
	     byte_unit(nic.second.ierrors),
	     byte(nic.second.oerrors),
	     byte_unit(nic.second.oerrors),
	     byte(nic.second.collisions),
	     byte_unit(nic.second.collisions),
	     byte(nic.second.drops),
	     byte_unit(nic.second.drops)
	     );
    first = false;
    tell_display(line++, LEFT, l);
  }
}

void
print(std::map<std::string, datum_t> data) {
  clear();

  unsigned line = 0;
  display_date(line);

  tell_display(line++, LEFT, "Cpu\t\t#\tUSER\tNICE\tSYSTEM\tINTERRUPT\tIDLE");
  // display cpus
  for (std::pair<std::string, datum_t> datum: data)
    cpu_view(line, datum.first, datum.second.cpus);
  line++;

  if (param.show_mem) {
    // display mem
    tell_display(line++, LEFT, "Mem\t\tREAL\t\tFREE\tSWAP");
    for (std::pair<std::string, datum_t> datum: data) {
      mem_view(line, datum.first, datum.second.mem);
    }
    line++;
  }

  if (param.show_procs) {
    tell_display(line++, LEFT, "Procs\t\tUTICKS\tSTICKS\tITICKS\tCPUSEC\tCPUCT\tPROCSZ\tRSSSZ\tPROC");
    for (std::pair<std::string, datum_t> datum: data)
      proc_view(line, datum.first, datum.second.procs);
    line++;
  }

  if (param.show_nics) {
    tell_display(line++, LEFT, "Nics\t\tIFACE\tIPKTS\tOPKTS\tIBYTES\tOBYTES\tIERR\tOERR\tCOLL\tDROPS");
    for (std::pair<std::string, datum_t> datum: data)
      show_nics(line, datum.first, datum.second.nics);
  }
  refresh();
}

double
todouble(std::string const& str) {
  return boost::lexical_cast<double>(str);
}

unsigned long long
toull(std::string const& str) {
  return boost::lexical_cast<unsigned long long>(str);
}

void
update_parameters() {
  param.height = LINES;
  param.width = COLS;

  while (true) {
    int key = getch();
    switch (key) {
    case 'c':
      param.detailed_cpu = !param.detailed_cpu;
      break;
    case 'i':
      param.show_nics = !param.show_nics;
    case 'm':
      param.show_mem = !param.show_mem;
      break;
    case 'n':
      param.show_nics = !param.show_nics;
      break;
    case 'p':
      param.show_procs = !param.show_procs;
      break;
    case 'q':
      endwin();
      std::exit(0);
    }
    if (key == ERR)
      break;
  }
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
      datum.cpus[cpu].interrupt = todouble(measure[6]);
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
    else if (measure[0] == "mem") {
      datum.mem.real_active = toull(measure[3]);
      datum.mem.real_total = toull(measure[4]);
      datum.mem.free = toull(measure[5]);
      datum.mem.swap_used = toull(measure[6]);
      datum.mem.swap_total = toull(measure[7]);
    }
    else {
      //std::cerr << "unknown measure: " << measure_raw << std::endl;
    }
  }
  
  data[host] = datum;
  update_parameters();
  print(data);
}

void
do_display(imsgbuf *ibuf) {
  {
    param.detailed_cpu = false;
    param.show_mem = true;
    param.show_procs = true;
    param.show_nics = false;
  }

  initscr();
  //raw();
  //keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);
  noecho();

  for (;;) {
    imsg imsg;
    int n;

    n = imsg_read(ibuf);
    if (n == -1 && errno != EAGAIN) {
      err(1, "imsg_read");
    }
    if (n == 0) {
      //std::cout << "imsg_read connection out" << std::endl;
      return;
    }

    for (;;) {
      n = imsg_get(ibuf, &imsg);
      if (n ==  -1)
	err(1, "imsg_get");
      if (n == 0) {
	break;
      }

      switch (imsg.hdr.type) {
      case IMSG_DATA: {
	// FIXME: Should handle corrupt data!
	std::string line((char *)imsg.data);
	treat_line(line);
	break;
      }
      default:
	//std::cout << "Unknown message" << std::endl;
	std::exit(42);
      }
      imsg_free(&imsg);
    }
  }
}
