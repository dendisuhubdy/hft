#ifndef STRATEGY_CONTAINER_H_
#define STRATEGY_CONTAINER_H_

#include <thread>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <string>

#include "core/base_strategy.h"
#include "util/sender.h"
#include "util/recver.h"

using namespace std;

class StrategyContainer {
 public:
  StrategyContainer(unordered_map<string, vector<BaseStrategy*> > &m);
  // explicit StrategyContainer(const StrategyContainer& sc) {}  // unable copy constructor
  // explicit StrategyContainer(StrategyContainer && sc) {}  // unable move constructor
  virtual ~StrategyContainer();
  void Start();
 private:
  static void RunCommandListener(unordered_map<string, vector<BaseStrategy*> > &m, Recver* command_recver);
  static void RunExchangeListener(unordered_map<string, vector<BaseStrategy*> > &m, Recver* exchangeinfo_recver);
  static void RunMarketDataListener(unordered_map<string, vector<BaseStrategy*> > &m, Recver* marketdata_recver);
  unordered_map<string, vector<BaseStrategy*> > &m;
  unique_ptr<Recver> marketdata_recver;
  unique_ptr<Recver> exchangeinfo_recver;
  unique_ptr<Recver> command_recver;
};

#endif  // STRATEGY_CONTAINER_H_
