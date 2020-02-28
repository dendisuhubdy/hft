#include <string.h>
#include <stdio.h>
#include <zmq.hpp>
#include <struct/order.h>
#include <util/recver.h>
#include <util/sender.h>
#include <util/dater.h>
#include <struct/market_snapshot.h>
#include <util/common_tools.h>
#include <core/base_strategy.h>
#include <core/strategy_container.h>
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
  libconfig::Config contract_config;
  std::string config_path = default_path + "/hft/config/prod/prod.config";
  std::string contract_config_path = default_path + "/hft/config/contract/contract.config";
  std::string time_config_path = default_path + "/hft/config/prod/time.config";
  param_cfg.readFile(config_path.c_str());
  contract_config.readFile(contract_config_path.c_str());
  std::unordered_map<std::string, std::vector<BaseStrategy*> > ticker_strat_map;
  TimeController tc(time_config_path);

  const libconfig::Setting & strategies = param_cfg.lookup("strategy");
  const libconfig::Setting & ticker_setting_map = contract_config.lookup("map");
  std::unordered_map<std::string, int> ticker_index_map;

  for (int i = 0; i < ticker_setting_map.getLength(); i++) {
    const libconfig::Setting & setting = ticker_setting_map[i];
    ticker_index_map[setting["ticker"]] = i;
  }

  std::unique_ptr<Sender> sender(new Sender("*:33333", "bind", "tcp"));
  std::unique_ptr<Sender> ordersender(new Sender("order_sub", "connect", "ipc", "core/order.dat"));
  Dater dt;
  std::string file = dt.GetValidFile(dt.GetCurrentDate(), -10);
  Contractor ct(file);

  for (int i = 0; i < strategies.getLength(); i++) {
    const libconfig::Setting & param_setting = strategies[i];
    std::string con = param_setting["unique_name"];
    bool no_close_today = false;
    if (param_setting.exists("no_close_today")) {
      no_close_today = param_setting["no_close_today"];
    }
    const libconfig::Setting & ticker_setting = ticker_setting_map[ticker_index_map[con]];
    auto s = new Strategy(param_setting, ticker_setting, tc, &ticker_strat_map, ct, sender.get(), ordersender.get(), "real", no_close_today);
    s->Print();
  }
  StrategyContainer sc(ticker_strat_map);
  sc.Start();
  HandleLeft();
  PrintResult();
}
