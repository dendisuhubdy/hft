#ifndef FEER_H_
#define FEER_H_

#include "common_tools.h"
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

class FEER {
 public:
  FEER(std::string config_path);
  ~FEER();
  Fee CalFee(std::string ticker, double open_price, int open_size, double close_price, int close_size);
  FeePoint CalFeePoint(std::string ticker, double open_price, int open_size, double close_price, int close_size);
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

#endif  // FEER_H_
