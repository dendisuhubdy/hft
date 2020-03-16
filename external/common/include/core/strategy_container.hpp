#ifndef STRATEGY_CONTAINER_HPP_
#define STRATEGY_CONTAINER_HPP_

#include <thread>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <string>

#include "core/base_strategy.h"
#include "util/recver.h"
#include "util/shm_reader.hpp"

using namespace std;

template<typename T>
class StrategyContainer {
 public:
  StrategyContainer(unordered_map<string, vector<BaseStrategy*> > &m)
    : m(m),
      marketdata_recver(new ShmReader<MarketSnapshot>("data_pub", 1000000)),
      exchangeinfo_recver(new T("exchange_info")),
      command_recver(new T("*:33334", "tcp", "bind")) {

}
  // explicit StrategyContainer(const StrategyContainer& sc) {}  // unable copy constructor
  // explicit StrategyContainer(StrategyContainer && sc) {}  // unable move constructor
  virtual ~StrategyContainer() {
  }
  void Start() {
    thread command_thread(RunCommandListener, std::ref(m), command_recver.get());
    thread exchangeinfo_thread(RunExchangeListener, std::ref(m), exchangeinfo_recver.get());
    thread marketdata_thread(RunMarketDataListener, std::ref(m), marketdata_recver.get());
    command_thread.join();
    exchangeinfo_thread.join();
    marketdata_thread.join();
  }
 private:
  static void RunCommandListener(unordered_map<string, vector<BaseStrategy*> > &m, T* command_recver) {
    while (true) {
      Command shot;
      command_recver->Recv(shot);
      printf("command recved!\n");
      shot.Show(stdout);
      string ticker = Split(shot.ticker, "|").front();
      if (ticker == "load_history") {
        // Load_history("mid.dat");
        continue;
      }
      vector<BaseStrategy*> sv = m[ticker];
      for (auto v : sv) {
        v->HandleCommand(shot);
      }
    }
  }


  static void RunExchangeListener(unordered_map<string, vector<BaseStrategy*> > &m, T* exchangeinfo_recver) {
    while (true) {
      ExchangeInfo info;
      exchangeinfo_recver->Recv(info);
      info.Show(stdout);
      std::vector<BaseStrategy*> sv = m[info.ticker];
      for (auto v : sv) {
        v->UpdateExchangeInfo(info);
      }
    }
  }

  static void RunMarketDataListener(unordered_map<string, vector<BaseStrategy*> > &m, T * marketdata_recver) {
    while (true) {
      MarketSnapshot shot;
      shot = marketdata_recver->read();
      auto sv = m[shot.ticker];
      for (auto s : sv) {
        s->UpdateData(shot);
      }
    }
  }
  unordered_map<string, vector<BaseStrategy*> > &m;
  unique_ptr<T> marketdata_recver;
  unique_ptr<T> exchangeinfo_recver;
  unique_ptr<T> command_recver;
};

#endif  // STRATEGY_CONTAINER_HPP_
