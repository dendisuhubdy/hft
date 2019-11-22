#include <string.h>
#include <stdio.h>
#include <zmq.hpp>
#include <order.h>
#include <recver.h>
#include <sender.h>
#include <Dater.h>
#include <market_snapshot.h>
#include <common_tools.h>
#include <base_strategy.h>
#include <strategy_container.h>
#include <unordered_map>

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <string>

#include "./strategy.h"

void HandleLeft() {
}

void PrintResult() {
}

int main() {
  std::string default_path = GetDefaultPath();
  libconfig::Config param_cfg;
  libconfig::Config ticker_cfg;
  std::string config_path = default_path + "/hft/config/prod/prod.config";
  std::string ticker_config_path = default_path + "/hft/config/contract/contract.config";
  // printf("config path is %s\n", config_path.c_str());
  param_cfg.readFile(config_path.c_str());
  ticker_cfg.readFile(ticker_config_path.c_str());
  const libconfig::Setting &sleep_time = param_cfg.lookup("time_controller")["sleep_time"];
  const libconfig::Setting &close_time = param_cfg.lookup("time_controller")["close_time"];
  const libconfig::Setting &force_close_time = param_cfg.lookup("time_controller")["force_close_time"];
  std::vector<std::string> sleep_time_v;
  std::vector<std::string> close_time_v;
  std::vector<std::string> force_close_time_v;
  for (int i = 0; i < sleep_time.getLength(); i++) {
    sleep_time_v.push_back(sleep_time[i]);
  }
  for (int i = 0; i < close_time.getLength(); i++) {
    close_time_v.push_back(close_time[i]);
  }
  for (int i = 0; i < force_close_time.getLength(); i++) {
    force_close_time_v.push_back(force_close_time[i]);
  }
  std::unordered_map<std::string, std::vector<BaseStrategy*> > ticker_strat_map;
  TimeController tc(sleep_time_v, close_time_v, force_close_time_v);

  const libconfig::Setting & strategies = param_cfg.lookup("strategy");
  const libconfig::Setting & ticker_setting_map = ticker_cfg.lookup("map");
  std::unordered_map<std::string, int> ticker_index_map;

  for (int i = 0; i < ticker_setting_map.getLength(); i++) {
    const libconfig::Setting & setting = ticker_setting_map[i];
    ticker_index_map[setting["ticker"]] = i;
  }
  std::string order_file_path = param_cfg.lookup("order_file");
  std::string exchange_file_path = param_cfg.lookup("exchange_file");
  std::string strat_file_path = param_cfg.lookup("strat_file");
  std::ofstream order_file(order_file_path.c_str(), ios::out | ios::app | ios::binary);
  std::ofstream exchange_file(exchange_file_path.c_str(), ios::out | ios::app | ios::binary);
  std::ofstream strat_file(strat_file_path.c_str(), ios::out | ios::app | ios::binary);

  Dater dt;
  std::string file = dt.GetValidFile(dt.GetCurrentDate(), -10);
  Contractor ct(file);
  std::unique_ptr<Sender> sender(new Sender("*:33333", "bind", "tcp"));
  std::vector<BaseStrategy*> sv;

  for (int i = 0; i < strategies.getLength(); i++) {
    const libconfig::Setting & param_setting = strategies[i];
    std::string con = param_setting["unique_name"];
    bool no_close_today = false;
    if (param_setting.exists("no_close_today")) {
      no_close_today = param_setting["no_close_today"];
    }
    const libconfig::Setting & ticker_setting = ticker_setting_map[ticker_index_map[con]];
    sv.emplace_back(new Strategy(param_setting, ticker_setting, tc, &ticker_strat_map, ct, sender.get(), "real", &order_file, &exchange_file, &strat_file, no_close_today));
  }
  StrategyContainer sc(ticker_strat_map);
  sc.Start();
  HandleLeft();
  PrintResult();
}
