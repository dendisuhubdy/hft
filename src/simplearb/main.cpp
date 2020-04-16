#include <string.h>
#include <stdio.h>
#include <zmq.hpp>
#include <struct/order.h>
#include <util/recver.hpp>
#include <util/sender.hpp>
#include <struct/market_snapshot.h>
#include <core/strategy_container.hpp>
#include <util/common_tools.h>
#include <core/base_strategy.h>
#include <util/history_worker.h>
#include <util/dater.h>
#include <util/shm_recver.hpp>
#include <util/shm_sender.hpp>
#include <thread>
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
  std::string config_path = default_path + "/hft/config/prod/prod.config";
  param_cfg.readFile(config_path.c_str());

  std::string time_config_path = default_path + "/hft/config/prod/time.config";
  TimeController tc(time_config_path);

  // std::unique_ptr<Sender<Order> > order_sender(new Sender<Order>("order_sub", "connect", "ipc", "order.dat"));
  std::unique_ptr<Sender<MarketSnapshot> > ui_sender(new Sender<MarketSnapshot>("*:33333", "bind", "tcp", "mid.dat"));
  std::unique_ptr<Sender<Order> > order_sender(new Sender<Order>("order_pub", "bind", "ipc", "order.dat"));

  HistoryWorker hw(Dater::FindOneValid(Dater::GetCurrentDate(), -20));
  std::unordered_map<std::string, std::vector<BaseStrategy*> > ticker_strat_map;
  std::string contract_config_path = default_path + "/hft/config/contract/contract.config";
  const libconfig::Setting & strategies = param_cfg.lookup("strategy");
  for (int i = 0; i < strategies.getLength(); i++) {
    const libconfig::Setting & param_setting = strategies[i];
    bool no_close_today = false;
    if (param_setting.exists("no_close_today")) {
      no_close_today = param_setting["no_close_today"];
    }
    auto s = new Strategy(param_setting, &ticker_strat_map, ui_sender.get(), order_sender.get(), &hw,  "real", no_close_today);
    s->Print();
  }

  StrategyContainer<Recver> sc(ticker_strat_map);
  sc.Start();
  HandleLeft();
  PrintResult();
}
