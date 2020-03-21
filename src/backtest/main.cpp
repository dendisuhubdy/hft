#include <libconfig.h++>
#include <unordered_map>
#include <map>
#include <utility>
#include <string>
#include <vector>

#include "core/backtester.h"
#include "util/ThreadPool.h"
#include "util/time_controller.h"
#include "util/sender.hpp"
#include "util/recver.hpp"
#include "util/shm_sender.hpp"
#include "util/shm_recver.hpp"
#include "util/dater.h"
#include "util/history_worker.h"
#include "util/common_tools.h"
#include "struct/market_snapshot.h"
#include "./strategy.h"

// std::unique_ptr<Sender<MarketSnapshot> > ui_sender(new Sender<MarketSnapshot>("*:33333", "bind", "tcp", "mid.dat"));
// std::unique_ptr<Sender<Order> > order_sender(new Sender<Order>("order_sub", "connect", "ipc", "order.dat"));
// std::unique_ptr<ShmSender<Order> > order_sender(new ShmSender<Order>("order_sub", 100000, "order.dat"));

std::map<std::string, std::string> GetBacktestFile() {
  std::string default_path = GetDefaultPath();

  libconfig::Config param_cfg;
  std::string config_path = default_path + "/hft/config/backtest/backtest.config";
  param_cfg.readFile(config_path.c_str());

  Dater dt;
  std::string start_date = param_cfg.lookup("start_date");
  std::string test_mode = param_cfg.lookup("test_mode");
  if (start_date == "today") {
    start_date = dt.GetDate();
  }
  if (start_date == "yesterday") {
    start_date = dt.GetDate("", -1);
  }
  int period = param_cfg.lookup("period");
  // return dt.GetDataFilesNameMapByDate(start_date, period);
  return dt.GetValidMap(start_date, period);
}

std::unordered_map<std::string, std::vector<BaseStrategy*> > GetStratMap(std::string date) {
  std::string default_path = GetDefaultPath();

  libconfig::Config param_cfg;
  std::string config_path = default_path + "/hft/config/backtest/backtest.config";
  param_cfg.readFile(config_path.c_str());

  std::string time_config_path = default_path + "/hft/config/prod/time.config";
  TimeController tc(time_config_path);

  std::unordered_map<std::string, std::vector<BaseStrategy*> > ticker_strat_map;
  std::string contract_config_path = default_path + "/hft/config/contract/contract.config";
  HistoryWorker hw(Dater::FindOneValid(date, -20));

  std::string ui_address = "backtest_ui_" + date;
  std::string order_address = "order_sub_" + date;
  std::string ui_file = "mid_" + date + ".dat";
  std::string order_file = "order_" + date + ".dat";
  // std::unique_ptr<Sender<MarketSnapshot> > ui_sender(new Sender<MarketSnapshot>(ui_address, "bind", "ipc", ui_file));
  // std::unique_ptr<Sender<Order> > order_sender(new Sender<Order>("order_sub", "connect", "ipc", "order.dat"));
  // std::unique_ptr<ShmSender<Order> > order_sender(new ShmSender<Order>(order_address, 100000, order_file));
  auto ui_sender = new Sender<MarketSnapshot>(ui_address, "bind", "ipc", ui_file);
  auto order_sender = new ShmSender<Order>(order_address, 100000, order_file);

  try {
    std::string test_mode = param_cfg.lookup("test_mode");
    const libconfig::Setting & strategies = param_cfg.lookup("strategy");
    for (int i = 0; i < strategies.getLength(); i++) {
      const libconfig::Setting & param_setting = strategies[i];
      bool no_close_today = false;
      if (param_setting.exists("no_close_today")) {
        no_close_today = param_setting["no_close_today"];
      }
      // auto s = new Strategy(param_setting, &ticker_strat_map, ui_sender.get(), order_sender.get(), &hw, test_mode, no_close_today);
      auto s = new Strategy(param_setting, &ticker_strat_map, ui_sender, order_sender, &hw, test_mode, no_close_today);
      s->Print();
    }
  } catch(const libconfig::SettingNotFoundException &nfex) {
    printf("Setting '%s' is missing", nfex.getPath());
    exit(1);
  } catch(const libconfig::SettingTypeException &tex) {
    printf("Setting '%s' has the wrong type", tex.getPath());
    exit(1);
  } catch (const std::exception& ex) {
    printf("EXCEPTION: %s\n", ex.what());
    exit(1);
  }
  return ticker_strat_map;
}

void RunBacktest(const std::string& date, const std::string& f) {
  auto tsm = GetStratMap(date);
  Backtester bt(tsm);
  bt.LoadData(f);
}

int main() {
  auto file_v = GetBacktestFile();
  ThreadPool pool(4);
  for (auto i: file_v) {
    pool.enqueue(RunBacktest, i.first, i.second);
  }
}
