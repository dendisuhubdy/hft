#ifndef BACKTESTER_H_
#define BACKTESTER_H_

#include "core/base_strategy.h"
#include "util/data_handler.hpp"

// thread_pool compatible
class Backtester : public DataHandler<MarketSnapshot> {
 public:
  Backtester(const std::unordered_map<std::string, std::vector<BaseStrategy*> > & m);

  ~Backtester();

  void HandleShot(MarketSnapshot* this_shot, MarketSnapshot* next_shot) override;

 private:
  BaseStrategy * bs;
  const std::unordered_map<std::string, std::vector<BaseStrategy*> > &tsm;
};

#endif // BACKTESTER_H_
