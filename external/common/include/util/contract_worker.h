#ifndef CONTRACT_WORKER_H_
#define CONTRACT_WORKER_H_

#include "util/common_tools.h"
#include "struct/order_side.h"
#include <unordered_map>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <assert.h>
#include <libconfig.h++>
#include <string>
#include <iostream>

struct Fee {
  double open_fee;
  double close_fee;
  Fee() : open_fee(0.0), close_fee(0.0) {
  }
};

struct FeePoint {
  double open_fee_point;
  double close_fee_point;
  FeePoint() : open_fee_point(0.0), close_fee_point(0.0) {
  }
};

class ContractWorker {
 public:
  explicit ContractWorker(const std::string& config_path, const std::string& key = "map");
  explicit ContractWorker(const libconfig::Setting & param_setting);
  ~ContractWorker();
  const libconfig::Setting& Lookup(const std::string& ticker);
  std::vector<std::string> GetTicker() const;
  Fee CalFee(std::string ticker, double open_price, int open_size, double close_price, int close_size, bool no_close_today = false);
  FeePoint CalFeePoint(std::string ticker, double open_price, int open_size, double close_price, int close_size, bool no_close_today = false);
  double CalPnl(std::string ticker, double open_price, int open_size, double close_price, int close_size, OrderSide::Enum close_side = OrderSide::Buy);
  double CalNetPnl(std::string ticker, float open_price, int open_size, float close_price, int close_size, OrderSide::Enum close_side = OrderSide::Buy, bool no_close_today = false);
  double GetMinPriceMove(std::string ticker);
  int GetConSize(std::string ticker);

  std::string GenContract(const std::string & pro, int year, int month);

  std::vector<std::string> GetActiveContracts(const std::string & pro, const std::string & date, int switch_day = 3);
 private:
  std::unordered_map<std::string, int> contract_size_map;
  std::unordered_map<std::string, double> min_price_move_map;
  std::unordered_map<std::string, double> deposit_rate_map;
  std::unordered_map<std::string, double> open_fee_rate_map;
  std::unordered_map<std::string, double> close_today_fee_rate_map;
  std::unordered_map<std::string, double> close_fee_rate_map;

  std::unordered_map<std::string, bool> is_fixed_open_fee_rate_map;
  std::unordered_map<std::string, bool> is_fixed_close_today_fee_rate_map;
  std::unordered_map<std::string, bool> is_fixed_close_fee_rate_map;

  std::unordered_map<std::string, double> open_fee_map;
  std::unordered_map<std::string, double> close_today_fee_map;
  std::unordered_map<std::string, double> close_fee_map;

  libconfig::Config cfg;
  const libconfig::Setting& setting;
  std::unordered_map<std::string, int> ticker_index_map;
  std::unordered_map<std::string, libconfig::Setting &> m_ticker_setting_map;
  std::unordered_map<std::string, std::string> exchange_map;
  
};

#endif  // CONTRACT_WORKER_H_
