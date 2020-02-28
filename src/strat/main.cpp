#include <string.h>
#include <stdio.h>
#include <zmq.hpp>
#include <struct/order.h>
#include <util/recver.h>
#include <util/sender.h>
#include <struct/market_snapshot.h>
#include <unordered_map>

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <string>

#include "strat/strategy.h"

void HandleLeft() {
}

void PrintResult() {
}

void* RunExchangeListener(void *param) {
  Strategy* st = reinterpret_cast<Strategy*>(param);
  Recver recver("exchange_info");
  while (true) {
    ExchangeInfo info;
    recver.Recv(info);
    st->UpdateExchangeInfo(info);
  }
  return NULL;
}

int main() {
  std::vector<std::string> sleep_time_v;
  sleep_time_v.emplace_back("10:14:00-10:30:00");
  sleep_time_v.emplace_back("11:29:20-13:30:00");
  sleep_time_v.emplace_back("00:58:20-09:00:00");
  TimeController tc(sleep_time_v, "21:00:00", "14:55:30");
  Recver data_recver("data");
  Strategy s("ni1905", "ni1901", 2, 10, tc, 1, "ni");
  pthread_t exchange_thread;
  if (pthread_create(&exchange_thread,
                     NULL,
                     &RunExchangeListener,
                     &s) != 0) {
    perror("pthread_create");
    exit(1);
  }
  while (true) {
    MarketSnapshot shot;
    data_recver.Recv(shot);
    // shot.Show(stdout, 1);
    shot.is_initialized = true;
    // ExecuteStrat(shot);
    s.UpdateData(shot);
  }
  HandleLeft();
  PrintResult();
}
