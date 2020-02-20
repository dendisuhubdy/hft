#include "timer.h"

Timer::Timer() {
}

Timer::~Timer() {
}

void Timer::StartTimer() {
  timeval current_time;
  gettimeofday(&current_time, NULL);
  timer_sec = current_time.tv_sec;
  timer_usec = current_time.tv_usec;
  is_timer_on = true;
}

void Timer::EndTimer(const std::string & label) {
  if (!is_timer_on) {
    printf("timer is not on, cant end!\n");
    return;
  }
  timeval current_time;
  gettimeofday(&current_time, NULL);
  printf("[%s]timer running time: %ld %ld\n", label.c_str(), current_time.tv_sec - timer_sec, current_time.tv_usec-timer_usec);
  is_timer_on = false;
}
