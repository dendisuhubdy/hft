#ifndef COMMAND_H_
#define COMMAND_H_

#include "define.h"
#include "command_type.h"
#include "offset.h"

#include <stdio.h>
#include <sys/time.h>
#include <fstream>

struct Command {
  timeval send_time;
  CommandType::Enum type;
  char ticker[MAX_CONTRACT_LENGTH];
  int vint[8];
  double vdouble[8];
  char tbd[128];

  Command() {
    snprintf(tbd, sizeof(tbd), "%s", "null");
  }

  bool Valid() const {
    if (type != CommandType::Uninited) {
      return true;
    }
    return false;
  }

  void Show(std::ofstream &stream) const {
    stream.write((char*)this, sizeof(*this));
    stream.flush();
  }

  void Show(FILE* stream, int depth=3) const {
    timeval show_time;
    gettimeofday(&show_time, NULL);
    fprintf(stream, "%ld %04ld %ld %04ld Command %s |",
            send_time.tv_sec, send_time.tv_usec, show_time.tv_sec, show_time.tv_usec, ticker);
    for (int i = 0; i < depth; i++) {
      fprintf(stream, "%d", vint[i]);
    }
    for (int i = 0; i < depth; i++) {
      fprintf(stream, "%.2f", vdouble[i]);
    }
  }

};

#endif  //  COMMAND_H_
