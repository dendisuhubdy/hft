#ifndef CALER_H_
#define CALER_H_

#include "common_tools.h"
#include "order_side.h"
#include <unordered_map>
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

class CALER {
 public:
  CALER(std::string config_path);
  CALER(const std::string& config_path, const std::string& key);
  CALER(const libconfig::Setting & param_setting);
  ~CALER();
  Fee CalFee(std::string ticker, double open_price, int open_size, double close_price, int close_size, bool no_close_today = false);
  FeePoint CalFeePoint(std::string ticker, double open_price, int open_size, double close_price, int close_size, bool no_close_today = false);
  double CalPnl(std::string ticker, double open_price, int open_size, double close_price, int close_size, OrderSide::Enum close_side = OrderSide::Buy);
  double CalNetPnl(std::string ticker, float open_price, int open_size, float close_price, int close_size, OrderSide::Enum close_side = OrderSide::Buy, bool no_close_today = false);
  double GetMinPriceMove(std::string ticker);
  int GetConSize(std::string ticker);
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
  
};

#endif  // CALER_H_
