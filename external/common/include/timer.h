#ifndef TIMER_H_
#define TIMER_H_

#include <unordered_map>
#include <time.h>
#include <libconfig.h++>
#include <sys/time.h>

#include <cmath>
#include <vector>
#include <iostream>
#include <string>

class Timer {
 public:
  Timer();
  ~Timer();

  void StartTimer();
  void EndTimer(const std::string & label="");

 private:
  long int timer_sec;
  long int timer_usec;
  bool is_timer_on;
};

#endif  // TIMER_H_
