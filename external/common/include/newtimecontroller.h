#ifndef NEWTIMECONTROLLER_H_
#define NEWTIMECONTROLLER_H_

#include "market_snapshot.h"
#include "order.h"
#include "sender.h"
#include "exchange_info.h"
#include "order_status.h"
#include "useful_function.h"
#include "time_status.h"
#include <unordered_map>
#include <time.h>
#include <libconfig.h++>
#include <sys/time.h>

#include <cmath>
#include <vector>
#include <iostream>
#include <string>

class TimeController {
 public:
  TimeController(string time_config);
  TimeController();
  TimeController(const TimeController & t);
  ~TimeController();

  TimeStatus::Enum ShotStatus(const MarketSnapshot& shot);
  TimeStatus::Enum CurrentStatus();
  TimeStatus::Enum IntStatus(int check_time) const;
  int TimevalInt(timeval t = {0,0}) const;

  void StartTimer();
  void EndTimer(const std::string & label="");

  std::string TimevalStr(timeval t = {0, 0}, bool show_time = false) const;
  std::string IntStr(int sec) const;

 private:
  int last_sec;
  int timezone_diff;
  std::vector<int>sleep_start;
  std::vector<int>sleep_stop;
  std::vector<int>close_start;
  std::vector<int>close_stop;
  std::vector<int>force_close_start;
  std::vector<int>force_close_stop;
  std::string mode;
  long int timer_sec;
  long int timer_usec;
  bool is_timer_on;

  bool IsMix(int s1, int e1, int s2, int e2) const;
  int Translate(const std::string & time) const;
  bool Check() const;
  void Push(std::vector<std::string> timestr, std::vector<int>& a, std::vector<int>& b);
};

#endif  // NEWTIMECONTROLLER_H_
