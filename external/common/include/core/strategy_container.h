#ifndef STRATEGY_CONTAINER_H_
#define STRATEGY_CONTAINER_H_

#include <thread>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <string>

#include "core/base_strategy.h"
#include "util/recver.hpp"
#include "util/shm_recver.hpp"

using namespace std;

class StrategyContainer {
 public:
  StrategyContainer(unordered_map<string, vector<BaseStrategy*> > &m);
  // explicit StrategyContainer(const StrategyContainer& sc) {}  // unable copy constructor
  // explicit StrategyContainer(StrategyContainer && sc) {}  // unable move constructor
  virtual ~StrategyContainer();
  void Start();
 private:
  static void RunCommandListener(unordered_map<string, vector<BaseStrategy*> > &m, Recver<Command>* crecver);
  static void RunExchangeListener(unordered_map<string, vector<BaseStrategy*> > &m, ShmRecver<ExchangeInfo>* erecver);
  static void RunMarketDataListener(unordered_map<string, vector<BaseStrategy*> > &m, ShmRecver<MarketSnapshot>* mrecver);
  unordered_map<string, vector<BaseStrategy*> > &m;
  unique_ptr<ShmRecver<MarketSnapshot> > marketdata_recver;
  unique_ptr<ShmRecver<ExchangeInfo> > exchangeinfo_recver;
  unique_ptr<Recver<Command> > command_recver;
};

#endif  // STRATEGY_CONTAINER_H_
