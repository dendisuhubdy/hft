#include <string.h>
#include <stdio.h>
#include <zmq.hpp>
#include <order.h>
#include <recver.h>
#include <sender.h>
#include <market_snapshot.h>
#include <unordered_map>

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <string>

#include "strat_MA/strategy.h"

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
  Recver pd_recver("pricer_sub");
  std::vector<std::string> strat_tickers;
  std::vector<std::string> topic_v;
  strat_tickers.emplace_back("ni1811");
  strat_tickers.emplace_back("hc1810");
  strat_tickers.emplace_back("ni1901");
  strat_tickers.emplace_back("zn1808");
  strat_tickers.emplace_back("zn1810");
  topic_v.emplace_back("MA10");
  topic_v.emplace_back("MA30");
  Strategy s(strat_tickers, topic_v, tc, "ma");
  pthread_t exchange_thread;
  if (pthread_create(&exchange_thread,
                     NULL,
                     &RunExchangeListener,
                     &s) != 0) {
    perror("pthread_create");
    exit(1);
  }
  while (true) {
    PricerData pd;
    pd_recver.Recv(pd);
    pd.Show(stdout);
    s.UpdateData(pd);
  }
  HandleLeft();
  PrintResult();
}
