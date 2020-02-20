#include <libconfig.h++>
#include <string>
#include <unordered_map>
#include <vector>

#include "core/backtester.h"
#include "util/ThreadPool.h"
#include "util/time_controller.h"
#include "util/sender.h"
#include "util/recver.h"
#include "util/dater.h"
#include "util/history_worker.h"
#include "/root/hft/src/simplearb/strategy.h"

int main() {
  std::string default_path = GetDefaultPath();

  libconfig::Config param_cfg;
  std::string config_path = default_path + "/hft/config/backtest/backtest.config";
  param_cfg.readFile(config_path.c_str());

  std::string time_config_path = default_path + "/hft/config/prod/time.config";
  TimeController tc(time_config_path);

  std::unique_ptr<Sender> ui_sender(new Sender("*:33333", "bind", "tcp", "mid.dat"));
  std::unique_ptr<Sender> order_sender(new Sender("order_sub", "connect", "ipc", "order.dat"));
  HistoryWorker hw(Dater::GetValidFile(Dater::GetCurrentDate(), -20));

  std::unordered_map<std::string, std::vector<BaseStrategy*> > ticker_strat_map;
  std::string contract_config_path = default_path + "/hft/config/contract/contract.config";

  try {
    const libconfig::Setting & strategies = param_cfg.lookup("strategy");
    for (int i = 0; i < strategies.getLength(); i++) {
      const libconfig::Setting & param_setting = strategies[i];
      bool no_close_today = false;
      if (param_setting.exists("no_close_today")) {
        no_close_today = param_setting["no_close_today"];
      }
      auto s = new Strategy(param_setting, &ticker_strat_map, ui_sender.get(), order_sender.get(), hw, "test", no_close_today);
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

  ThreadPool pool(4);
  std::vector<std::string> file_list = {"/running/2020-02-07/future2020-02-07.dat.gz"};
  for (auto f: file_list) {
    pool.enqueue([](const std::unordered_map<std::string, std::vector<BaseStrategy*> >& m, std::string f) {Backtester bt(m); bt.LoadData(f);}, ticker_strat_map, f);
  }
}
