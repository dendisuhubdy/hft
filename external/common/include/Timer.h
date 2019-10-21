//time_interval.h
#pragma once

#define GCC 1

#include <iostream>
#include <memory>
#include <string>
#ifdef GCC
#include <sys/time.h>
#else
#include <ctime>
#endif // GCC

class TimeInterval {
 public:
  TimeInterval(const std::string& d) : detail(d) {
      init();
  }

  TimeInterval() {
      init();
  }

  ~TimeInterval() {
    #ifdef GCC
      gettimeofday(&end, NULL);
      std::cout << detail 
        << (end.tv_sec - start.tv_sec) << "ms and " <<  (end.tv_usec - start.tv_usec)
        << " us" << std::endl;
    #else
      end = clock();
      std::cout << detail 
        << (double)(end - start) << " ms" << std::endl;
    #endif // GCC
  }

 protected:
   void init() {
     #ifdef GCC
       gettimeofday(&start, NULL);
     #else
       start = clock();
     #endif // GCC
   }
 private:
   std::string detail;
   #ifdef GCC
     timeval start, end;
   #else
     clock_t start, end;
   #endif // GCC
};

#define TIMER_RUN(d)   std::shared_ptr<TimeInterval> time_interval_scope_begin = std::make_shared<TimeInterval>(d)
