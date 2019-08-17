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

void Load_history(std::string file_name) {
  std::unique_ptr<Sender> sender(new Sender("*:33336", "bind", "tcp"));
  std::ifstream raw_file;
  raw_file.open(file_name.c_str(), ios::in|ios::binary);
  if (!raw_file) {
    printf("%s is not existed!", file_name.c_str());
    return;
  }
  MarketSnapshot shot;
  while (raw_file.read(reinterpret_cast<char *>(&shot), sizeof(shot))) {
    sender.get()->Send(shot);
  }
  MarketSnapshot fshot;
  snprintf(fshot.ticker, sizeof(fshot.ticker), "%s", "load_end");
  sender.get()->Send(fshot);
  printf("Load_history finished!\n");
}

void* RunCommandListener(void *param) {
  std::unordered_map<std::string, std::vector<BaseStrategy*> > * sv_map = reinterpret_cast<std::unordered_map<std::string, std::vector<BaseStrategy*> >* >(param);
  Recver recver("*:33334", "tcp", "bind");
  while (true) {
    MarketSnapshot shot;
    shot = recver.Recv(shot);
    printf("command recved!\n");
    shot.Show(stdout);
    std::string ticker = Split(shot.ticker, "|").front();
    if (ticker == "load_history") {
      Load_history("mid.dat");
    }
    std::vector<BaseStrategy*> sv = (*sv_map)[ticker];
    for (auto v : sv) {
      v->HandleCommand(shot);
    }
  }
  return NULL;
}

void* RunExchangeListener(void *param) {
  std::unordered_map<std::string, std::vector<BaseStrategy*> > * sv_map = reinterpret_cast<std::unordered_map<std::string, std::vector<BaseStrategy*> >* >(param);
  Recver recver("exchange_info");
  while (true) {
    ExchangeInfo info;
    info = recver.Recv(info);
    info.Show(stdout);
    std::vector<BaseStrategy*> sv = (*sv_map)[info.contract];
    for (auto v : sv) {
      v->UpdateExchangeInfo(info);
    }
  }
  return NULL;
}

int main() {
  std::string default_path = GetDefaultPath();
  libconfig::Config param_cfg;
  libconfig::Config contract_cfg;
  std::string config_path = default_path + "/hft/config/backtest/backtest.config";
  std::string contract_config_path = default_path + "/hft/config/backtest/contract.config";
  printf("path is %s\n", config_path.c_str());
  param_cfg.readFile(config_path.c_str());
  contract_cfg.readFile(contract_config_path.c_str());
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
  Recver data_recver("data_pub");

  const libconfig::Setting & strategies = param_cfg.lookup("strategy");
  const libconfig::Setting & contract_setting_map = contract_cfg.lookup("map");
  std::unordered_map<std::string, int> contract_index_map;

  for (int i = 0; i < contract_setting_map.getLength(); i++) {
    const libconfig::Setting & setting = contract_setting_map[i];
    contract_index_map[setting["ticker"]] = i;
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
    const libconfig::Setting & contract_setting = contract_setting_map[contract_index_map[con]];
    sv.emplace_back(new Strategy(param_setting, contract_setting, tc, &ticker_strat_map, ct, sender.get(), "real", &order_file, &exchange_file, &strat_file, no_close_today));
  }

  printf("strategy init all ok!\n");
  pthread_t command_thread;
  if (pthread_create(&command_thread,
                     NULL,
                     &RunCommandListener,
                     &ticker_strat_map) != 0) {
    perror("command_pthread_create");
    exit(1);
  }

  pthread_t exchange_thread;
  if (pthread_create(&exchange_thread,
                     NULL,
                     &RunExchangeListener,
                     &ticker_strat_map) != 0) {
    perror("pthread_create");
    exit(1);
  }
  sleep(3);
  printf("send query position ok!\n");
  while (true) {
    MarketSnapshot shot;
    shot = data_recver.Recv(shot);
    shot.is_initialized = true;
    std::vector<BaseStrategy*> sv = ticker_strat_map[shot.ticker];
    for (auto v : sv) {
      v->UpdateData(shot);
    }
  }
  HandleLeft();
  PrintResult();
}
