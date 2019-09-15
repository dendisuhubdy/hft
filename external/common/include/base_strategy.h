#ifndef BASE_STRATEGY_H_
#define BASE_STRATEGY_H_

#include "market_snapshot.h"
#include "order.h"
#include "sender.h"
#include "define.h"
#include "exchange_info.h"
#include "order_status.h"
#include "common_tools.h"
#include "strategy_status.h"
#include "Contractor.h"
#include "timecontroller.h"
#include <unordered_map>

#include <cmath>
#include <vector>
#include <string>
#include <unistd.h>
#include <memory>

class BaseStrategy {
 public:
  BaseStrategy();
      
  virtual ~BaseStrategy();

  virtual void Start() = 0;
  virtual void Stop() = 0;
  void UpdateData(const MarketSnapshot& shot);
  void UpdateExchangeInfo(const ExchangeInfo& info);
  void RequestQryPos();
  virtual void Print() const;
  virtual void Init() = 0;
  /*
  virtual void InitTicker() = 0;
  virtual void InitTimer() = 0;
  virtual void InitFile() = 0;
  */
  void SendPlainText(const std::string & flag, const std::string & s);

  virtual void Clear();
  void debug() const;
  double GetMid(const std::string & ticker);
  void UpdateCT(const Contractor& ct);
  virtual void UpdateTicker();
  virtual void HandleCommand(const MarketSnapshot& shot);
 protected:
  void UpdateAvgCost(const std::string & contract, double trade_price, int size);
  std::string GenOrderRef();
  Order* NewOrder(const std::string & contract, OrderSide::Enum side, int size, bool control_price, bool sleep_order, const std::string & tbd, bool no_today = false);
  void ModOrder(Order* o, bool sleep=false);
  void CancelAll(const std::string & contract);
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
  
  bool position_ready;
  Sender* order_sender;
  Sender* ui_sender;
  unordered_map<std::string, MarketSnapshot> shot_map;
  unordered_map<std::string, Order*> order_map;
  unordered_map<std::string, Order*> sleep_order_map;
  unordered_map<std::string, int> position_map;
  unordered_map<std::string, double> avgcost_map;
  int ref_num;
  pthread_mutex_t cancel_mutex;
  pthread_mutex_t order_ref_mutex;
  pthread_mutex_t mod_mutex;
  std::ofstream* order_file;
  std::ofstream* exchange_file;
  std::ofstream* strat_file;
  std::string m_strat_name;
  TimeController* m_tc;
  int contract_size;
  std::unordered_map<std::string, int> cancel_map;
  StrategyStatus::Enum ss;
  std::unordered_map<std::string, bool> ticker_map;
  MarketSnapshot last_shot;
  long int build_position_time;
  int max_holding_sec;
  Contractor m_ct;
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
  virtual void ModerateOrders(const std::string & contract);
  virtual bool PriceChange(double current_price, double reasonable_price, OrderSide::Enum side, double edurance);
  virtual void DoOperationAfterUpdatePos(Order* o, const ExchangeInfo& info);
  virtual void DoOperationAfterUpdateData(const MarketSnapshot& shot);
  virtual void DoOperationAfterFilled(Order* o, const ExchangeInfo& info);

  virtual double OrderPrice(const std::string & contract, OrderSide::Enum side, bool control_price) = 0;
};

#endif  // BASE_STRATEGY_H_
