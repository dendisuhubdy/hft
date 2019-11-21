#ifndef SRC_BACKTEST_ORDER_HANDLER_H_
#define SRC_BACKTEST_ORDER_HANDLER_H_

#include <order_side.h>
#include <order.h>
#include <stdio.h>
#include <Python.h>
#include <unordered_map>
#include <common_tools.h>
#include <sender.h>
#include <caler.h>
#include <exchange_info.h>

#include <libconfig.h++>

#include <stdexcept>
#include <string>
#include <memory>


class OrderHandler {
 public:
  OrderHandler();
  ~OrderHandler();

  bool Handle(const Order & order);

 private:
  void HandleConfig(const Order & order);
  void HandleContractConfig(const Order & order);
  void HandleNew(const Order & order);
  // void HandleCancel(const Order & order);
  // void HandleMod(const Order & order);
  void HandleDate(std::string s);
  void HandleLeft();
  void GenBackTestReport();
  void GenDayReport();
  void Clear();

  void Plot();
  // Fee CalFee(std::string ticker, double open_price, double close_price, int close_size);

  void SendFakeFilledInfoBack(const Order & order);

  std::unordered_map<std::string, double> fozen_capital_map;

  std::unordered_map<std::string, std::string> ticker_strat_map;

  std::unordered_map<std::string, double> avgcost_map;
  std::unordered_map<std::string, int> current_pos_map;
  std::unordered_map<std::string, double> gross_pnl_map;
  std::unordered_map<std::string, double> net_pnl_map;
  std::unordered_map<std::string, double> product_gross_pnl_map;
  std::unordered_map<std::string, double> product_net_pnl_map;

  std::unordered_map<std::string, int> close_size_map;
  std::unordered_map<std::string, int> force_close_map;
  std::unordered_map<std::string, double> fee_map;

  std::unordered_map<std::string, double> cum_avgcost_map;
  std::unordered_map<std::string, int> cum_left_pos_map;

  std::unordered_map<std::string, int> cum_close_size_map;
  std::unordered_map<std::string, int> cum_force_close_map;

  std::unordered_map<std::string, double> cum_fee_map;
  std::unordered_map<std::string, double> cum_gross_pnl_map;
  std::unordered_map<std::string, double> cum_net_pnl_map;
  std::unordered_map<std::string, double> cum_product_gross_pnl_map;
  std::unordered_map<std::string, double> cum_product_net_pnl_map;

  std::unordered_map<std::string, int> sum_left_map;

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

  std::unordered_map<std::string, double> min_price_move_map;
  std::unordered_map<std::string, int> ticker_size_map;
  FILE* record_file;
  std::string date;
  std::string main_ticker;
  std::string hedge_ticker;
  Sender * sender;
  CALER * caler;
  // std::unordered_map<int, double> round_pnl;
};

#endif  // SRC_BACKTEST_ORDER_HANDLER_H_
