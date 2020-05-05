#ifndef SRC_SPREADARB_STRATEGY_H_
#define SRC_SPREADARB_STRATEGY_H_

#include <struct/market_snapshot.h>
#include <struct/strategy_status.h>
#include <util/time_controller.h>
#include <util/contractor.h>
#include <struct/order.h>
#include <struct/command.h>
#include <util/zmq_sender.hpp>
#include <util/zmq_sender.hpp>
#include <util/caler.h>
#include <util/dater.h>
#include <struct/exchange_info.h>
#include <struct/order_status.h>
#include <util/history_worker.h>
#include <util/common_tools.h>
#include <core/base_strategy.h>
#include <libconfig.h++>
#include <unordered_map>

#include <cmath>
#include <vector>
#include <string>
#include <iostream>
#include <memory>

class Strategy : public BaseStrategy {
 public:
  explicit Strategy(const libconfig::Setting & param_setting, std::unordered_map<std::string, std::vector<BaseStrategy*> >*ticker_strat_map, ZmqSender<MarketSnapshot>* uisender, ZmqSender<Order>* ordersender, HistoryWorker* hw, const std::string & mode = "real", bool no_close_today = false);
  ~Strategy();

  void Start() override;
  void Stop() override;

  // void Clear() override;
  void HandleCommand(const Command& shot) override;
  // void UpdateTicker() override;
 private:
  bool FillStratConfig(const libconfig::Setting& param_setting, bool no_close_today);
  void RunningSetup(std::unordered_map<std::string, std::vector<BaseStrategy*> >*ticker_strat_map, ZmqSender<MarketSnapshot>* uisender, ZmqSender<Order>* ordersender, const std::string & mode);
  void ClearPositionRecord();
  void DoOperationAfterUpdateData(const MarketSnapshot& shot) override;
  void DoOperationAfterUpdatePos(Order* o, const ExchangeInfo& info) override;
  void DoOperationAfterFilled(Order* o, const ExchangeInfo& info) override;
  void DoOperationAfterCancelled(Order* o) override;
  void ModerateOrders(const std::string & contract) override;

  void Init() override;
  bool Ready() override;
  void Pause() override;
  void Resume() override;
  void Run() override;
  void Train() override;
  void Flatting() override;

  void UpdateBuildPosTime();

  double OrderPrice(const std::string & contract, OrderSide::Enum side, bool control_price) override;

  OrderSide::Enum OpenLogicSide();
  bool OpenLogic();
  void CloseLogic();

  void Open(OrderSide::Enum side);
  bool Close(bool force_flat = false);

  void CalParams();
  bool HitMean();

  double GetPairMid();

  void ForceFlat() override;

  bool Spread_Good();

  bool IsAlign();

  void UpdateBound(OrderSide::Enum side);
  void StopLossLogic();
  void HandleTestOrder(Order *o);

  char order_ref[MAX_ORDERREF_SIZE];
  std::string main_ticker;
  std::string hedge_ticker;
  int max_pos;
  double min_price_move;

  // std::unordered_map<std::string, std::vector<BaseStrategy*> >*tsm;
  int cancel_limit;
  std::unordered_map<std::string, double> mid_map;
  double up_diff;
  double down_diff;
  double range_width;
  double mean;
  std::vector<double> map_vector;
  std::vector<double> long_mid;
  std::vector<double> short_mid;
  int current_pos;
  double min_profit;
  unsigned int min_train_sample;
  double min_range;
  double increment;
  std::string mode;
  double spread_threshold;
  int closed_size;
  double last_valid_mid;
  double stop_loss_up_line;
  double stop_loss_down_line;
  int max_loss_times;
  double stop_loss_times;
  double stop_loss_margin;
  int max_close_try;
  double current_spread;
  bool is_started;
  bool no_close_today;
  // int open_count;
  // int close_count;
  HistoryWorker* m_hw;
  int max_round;
  int close_round;
};

#endif  // SRC_SPREADARB_STRATEGY_H_
