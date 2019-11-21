#ifndef STRATEGY_CONTAINER_H_
#define STRATEGY_CONTAINER_H_

#include <thread>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <string>

#include "base_strategy.h"
#include "sender.h"
#include "recver.h"

using namespace std;

class StrategyContainer {
 public:
  StrategyContainer(unordered_map<string, vector<BaseStrategy*> > &m, Recver & marketdata_recver,
                    Recver & exchangeinfo_recver, Recver & command_recver);
  // explicit StrategyContainer(const StrategyContainer& sc) {}  // unable copy constructor
  // explicit StrategyContainer(StrategyContainer && sc) {}  // unable move constructor
  virtual ~StrategyContainer();
  void Start();
 private:
  static void RunCommandListener(unordered_map<string, vector<BaseStrategy*> > &m, Recver & command_recver);
  static void RunExchangeListener(unordered_map<string, vector<BaseStrategy*> > &m, Recver & exchangeinfo_recver);
  static void RunMarketDataListener(unordered_map<string, vector<BaseStrategy*> > &m, Recver & marketdata_recver);
  unordered_map<string, vector<BaseStrategy*> > &m;
  Recver & marketdata_recver;
  Recver & exchangeinfo_recver;
  Recver & command_recver;
};

#endif  // STRATEGY_CONTAINER_H_
