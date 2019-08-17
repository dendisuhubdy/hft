#ifndef PNLER_H_
#define PNLER_H_

#include "common_tools.h"
#include "feer.h"
#include "order_side.h"
#include <unordered_map>
#include <libconfig.h++>
#include <string>
#include <iostream>

class PNLER {
 public:
  PNLER(std::string config_path);
  ~PNLER();
  double CalPnl(std::string ticker, double open_price, int open_size, double close_price, int close_size, OrderSide::Enum close_side);
  double CalNetPnl(std::string ticker, double open_price, int open_size, double close_price, int close_size, OrderSide::Enum close_side);
 private:
  FEER* feer;
  /*
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
  */
};

#endif  // PNLER_H_
