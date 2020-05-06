#ifndef BASE_STRATEGY_H_
#define BASE_STRATEGY_H_

#include "struct/market_snapshot.h"
#include "struct/order.h"
#include "util/base_sender.hpp"
#include "util/base_sender.hpp"
#include "util/dater.h"
#include "define.h"
#include "struct/command.h"
#include "struct/exchange_info.h"
#include "struct/order_status.h"
// #include "util/caler.h"
#include "util/common_tools.h"
#include "struct/strategy_status.h"
// #include "util/contractor.h"
#include "util/time_controller.h"
#include "util/history_worker.h"
#include "util/contract_worker.h"
#include <libconfig.h++>
#include <unordered_map>

#include <cmath>
#include <vector>
#include <string>
#include <unistd.h>
#include <memory>
#include <mutex>

class BaseStrategy {
 public:
  BaseStrategy();
      
  virtual ~BaseStrategy();

  virtual void Start() = 0;
  virtual void Stop() = 0;
  void UpdateData(const MarketSnapshot& shot);
  void UpdateData(const MarketSnapshot& this_shot, const MarketSnapshot& next_shot);
  void UpdateExchangeInfo(const ExchangeInfo& info);
  void RequestQryPos();
  virtual void Print() const;
  virtual void Init() = 0;
  void SendPlainText(const std::string & flag, const std::string & s);

  virtual void Clear();
  void debug() const;
  double GetMid(const std::string & ticker);
  virtual void UpdateTicker();
  virtual void HandleCommand(const Command& shot);
 protected:
  void UpdateAvgCost(const std::string & ticker, double trade_price, int size);
  std::string GenOrderRef();
  Order* NewOrder(const std::string & ticker, OrderSide::Enum side, int size, bool control_price, bool sleep_order, const std::string & tbd, bool no_today = false);
  void ModOrder(Order* o, bool sleep=false);
  void CancelAll(const std::string & ticker);
  void CancelAll();
  void CancelOrder(Order* o);
  void DelOrder(const std::string & ref);
  void DelSleepOrder(const std::string & ref);
  void ClearAll();
  void Wakeup();
  void Wakeup(Order* o);
  void CheckStatus(const MarketSnapshot& shot);

  bool TimeUp() const;

  void UpdatePos(Order* o, const ExchangeInfo& info);

  void LoadPosition();
  
  bool position_ready;
  BaseSender<Order>* order_sender;
  BaseSender<MarketSnapshot>* ui_sender;
  unordered_map<std::string, MarketSnapshot> shot_map;
  unordered_map<std::string, MarketSnapshot> next_shot_map;
  unordered_map<std::string, Order*> order_map;
  unordered_map<std::string, Order*> sleep_order_map;
  unordered_map<std::string, int> position_map;
  unordered_map<std::string, double> avgcost_map;
  int ref_num;
  std::mutex cancel_mutex;
  std::mutex order_ref_mutex;
  std::mutex mod_mutex;
  std::string m_strat_name;
  TimeController* m_tc;
  int ticker_size;
  std::unordered_map<std::string, int> cancel_map;
  StrategyStatus::Enum ss;
  std::unordered_map<std::string, bool> ticker_map;
  MarketSnapshot last_shot;
  long int build_position_time;
  int max_holding_sec;
  ContractWorker * m_cw;
  HistoryWorker m_hw;
  bool init_ticker;
 private:
  virtual void DoOperationAfterCancelled(Order* o);
  virtual void Run() = 0;
  virtual void Resume() = 0;
  virtual void Pause() = 0;
  virtual void Train();
  virtual void Flatting();
  virtual void ForceFlat();

  virtual bool Ready() = 0;
  virtual void ModerateOrders(const std::string & ticker);
  virtual bool PriceChange(double current_price, double reasonable_price, OrderSide::Enum side, double edurance);
  virtual void DoOperationAfterUpdatePos(Order* o, const ExchangeInfo& info);
  virtual void DoOperationAfterUpdateData(const MarketSnapshot& shot);
  virtual void DoOperationAfterFilled(Order* o, const ExchangeInfo& info);

  virtual double OrderPrice(const std::string & ticker, OrderSide::Enum side, bool control_price) = 0;
};

#endif  // BASE_STRATEGY_H_
