#ifndef BACKTESTER_H_
#define BACKTESTER_H_

#include "core/base_strategy.h"
#include "util/data_handler.h"

// thread_pool compatible
class Backtester : public DataHandler {
 public:
  Backtester(const std::unordered_map<std::string, std::vector<BaseStrategy*> > & m);

  ~Backtester();

  void HandleShot(MarketSnapshot* shot) override;

 private:
  BaseStrategy * bs;
  const std::unordered_map<std::string, std::vector<BaseStrategy*> > &tsm;
};

#endif // BACKTESTER_H_
