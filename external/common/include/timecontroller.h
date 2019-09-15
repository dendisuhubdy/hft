#ifndef TIMECONTROLLER_H_
#define TIMECONTROLLER_H_

#include "market_snapshot.h"
#include "order.h"
#include "sender.h"
#include "exchange_info.h"
#include "order_status.h"
#include "useful_function.h"
#include "time_status.h"
#include <unordered_map>
#include <time.h>
#include <sys/time.h>

#include <cmath>
#include <vector>
#include <iostream>
#include <string>

class TimeController {
 public:
  explicit TimeController(std::vector<std::string>sleep_time, std::vector<std::string> close_time, std::vector<std::string> force_close, const std::string & mode="real");
  TimeController();
  TimeController(const TimeController & t);

  ~TimeController();

  TimeStatus::Enum CheckCurrentTime(const MarketSnapshot& shot);

  TimeStatus::Enum CheckTime(int check_time) const;

  int GetTodaySec();
  int GetSec(timeval t = {0,0}) const;
  bool IsMix(int s1, int e1, int s2, int e2) const;
  void StartTimer();
  void EndTimer(const std::string & label="");
  int GetStratSec(timeval t = {0, 0}) const;
  std::string TimeToStr(timeval t = {0, 0}, bool show_time = false) const;

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

  int Translate(const std::string & time) const;
  bool Check() const;
  void Push(std::vector<std::string> timestr, std::vector<int>& a, std::vector<int>& b);
};

#endif  // TIMECONTROLLER_H_
