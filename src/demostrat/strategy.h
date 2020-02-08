#ifndef SRC_DEMOSTRAT_STRATEGY_H_
#define SRC_DEMOSTRAT_STRATEGY_H_

#include <market_snapshot.h>
#include <time_controller.h>
#include <order.h>
#include <sender.h>
#include <exchange_info.h>
#include <order_status.h>
#include <common_tools.h>
#include <base_strategy.h>
#include <unordered_map>

#include <cmath>
#include <vector>
#include <string>


class Strategy : public BaseStrategy {
 public:
  explicit Strategy(std::unordered_map<std::string, std::vector<BaseStrategy*> >*ticker_strat_map);
  ~Strategy();

  // must realize
  void Start() override;
  void Stop() override;
 private:
  // not must realize, but usually, it should
  void DoOperationAfterUpdatePos(Order* o, const ExchangeInfo& info) override;
  void DoOperationAfterUpdateData(const MarketSnapshot& shot) override;
  void DoOperationAfterFilled(Order* o, const ExchangeInfo& info) override;
  void DoOperationAfterCancelled(Order* o) override;

  // not must
  void ModerateOrders(const std::string & contract) override;

  void Init() override;
  bool Ready() override;
  void Pause() override;
  void Resume() override;
  void Run() override;
  void Train() override;
  void Flatting() override;

  // must realize, define the order price logic when send an order
  double OrderPrice(const std::string & contract, OrderSide::Enum side, bool control_price) override;

  std::string main_ticker;
  std::string hedge_ticker;
};

#endif  // SRC_DEMOSTRAT_STRATEGY_H_
