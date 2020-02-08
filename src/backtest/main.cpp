#include <string.h>
#include <zlib.h>
#include <stdio.h>
#include <zmq.hpp>
#include <libconfig.h++>
#include <order.h>
#include <define.h>
#include <recver.h>
#include <sender.h>
#include <Dater.h>
#include <pthread.h>
#include <Contractor.h>
#include <market_snapshot.h>
#include <common_tools.h>
#include <base_strategy.h>
#include <sys/time.h>
#include <sys/sysinfo.h>
#include <unordered_map>

#include <iostream>
#include <cctype>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <string>

#include "backtest/strategy.h"
#include "backtest/order_handler.h"

void HandleLeft() {
  return;
}

void PrintResult() {
  return;
}

void* RunExchangeListener(void *param) {
  std::unordered_map<std::string, std::vector<BaseStrategy*> > * sv_map = reinterpret_cast<std::unordered_map<std::string, std::vector<BaseStrategy*> >* >(param);
  Recver recver("exchange_info");
  while (true) {
    ExchangeInfo info;
    recver.Recv(info);
    std::vector<BaseStrategy*> sv = (*sv_map)[info.ticker];
    for (auto v : sv) {
      v->UpdateExchangeInfo(info);
    }
  }
  return NULL;
}

void* RunCtpOrderListener(void *param) {
  Recver* r = new Recver("order_pub");
  std::shared_ptr<Sender> sender(new Sender("*:33335", "bind", "tcp"));
  while (true) {
    Order o;
    r->Recv(o);
    sender.get()->Send(o);
  }
  return NULL;
}

void* RunProxy(void *param) {
  zmq::context_t context(1);
  zmq::socket_t sub(context, ZMQ_XSUB);
  sub.bind("ipc://order_sub");
  zmq::socket_t pub(context, ZMQ_XPUB);
  pub.bind("ipc://order_pub");
  zmq::proxy(sub, pub, NULL);
  return NULL;
}

void* RunOrderListener(void *param) {
  OrderHandler *oh = reinterpret_cast<OrderHandler*>(param);
  Recver recver("order_pub");
  std::shared_ptr<Sender> sender(new Sender("*:33335", "bind", "tcp"));
  while (true) {
    Order o;
    recver.Recv(o);
    sender.get()->Send(o);
    if (!oh->Handle(o)) {
      printf("order failed!\n");
      o.Show(stdout);
      return NULL;
    }
  }
  return NULL;
}

void Load_history(std::string file_name) {
  std::unique_ptr<Sender> sender(new Sender("*:33336", "bind", "tcp"));
  std::ifstream raw_file;
  raw_file.open(file_name.c_str(), ios::in|ios::binary);
  if (!raw_file) {
    printf("%s is not existed!\n", file_name.c_str());
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
    Command shot;
    recver.Recv(shot);
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

int main() {
  std::string default_path = GetDefaultPath();
  libconfig::Config param_cfg;
  libconfig::Config ticker_cfg;
  std::string param_config_path = default_path + "/hft/config/backtest/backtest.config";
  std::string ticker_config_path = default_path + "/hft/config/contract/bk_contract.config";
  std::string time_config_path = default_path + "/hft/config/prod/time.config";
  param_cfg.readFile(param_config_path.c_str());
  ticker_cfg.readFile(ticker_config_path.c_str());
  try {
    std::unordered_map<std::string, std::vector<BaseStrategy*> > ticker_strat_map;
    TimeController tc(time_config_path);

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
    std::ofstream order_file(order_file_path.c_str(), ios::out | ios::binary);
    std::ofstream exchange_file(order_file_path.c_str(), ios::out | ios::binary);
    std::ofstream strat_file(strat_file_path.c_str(), ios::out | ios::binary);

    std::string matcher_mode = param_cfg.lookup("matcher_mode");
    pthread_t command_thread;
    if (pthread_create(&command_thread,
                       NULL,
                       &RunCommandListener,
                       &ticker_strat_map) != 0) {
      perror("command_pthread_create");
      exit(1);
    }
    /*
    pthread_t loadui_thread;
    if (pthread_create(&loadui_thread,
                       NULL,
                       &RunUiListener,
                       NULL) != 0) {
      perror("ui_pthread_create");
      exit(1);
    }
    */
    pthread_t proxy_thread;
    if (pthread_create(&proxy_thread,
                       NULL,
                       &RunProxy,
                       NULL) != 0) {
      perror("ui_pthread_create");
      exit(1);
    }
    pthread_t ctporder_thread;
    if (pthread_create(&ctporder_thread,
                       NULL,
                       &RunCtpOrderListener,
                       NULL) != 0) {
      perror("ui_pthread_create");
      exit(1);
    }
    if (matcher_mode == "c++") {
      OrderHandler oh;
      printf("start exchange thread\n");
      pthread_t exchange_thread;
      if (pthread_create(&exchange_thread,
                         NULL,
                         &RunExchangeListener,
                         &ticker_strat_map) != 0) {
        perror("exchange_pthread_create");
        exit(1);
      }
      sleep(1);

      pthread_t order_thread;
      if (pthread_create(&order_thread,
                         NULL,
                         &RunOrderListener,
                         &oh) != 0) {
        perror("order_pthread_create");
        exit(1);
      }
    }
    sleep(1);
    Dater dt;
    std::string start_date = param_cfg.lookup("start_date");
    if (start_date == "today") {
      start_date = dt.GetDate();
    }
    if (start_date == "yesterday") {
      start_date = dt.GetDate("", -1);
    }
    int period = param_cfg.lookup("period");
    std::vector<std::string> file_v = dt.GetDataFilesNameByDate(start_date, period, true);
    Contractor ct(dt.GetValidFile(start_date, -40));
    PrintVector(ct.GetAllTick());
    std::unique_ptr<Sender> sender(new Sender("*:33333", "bind", "tcp", "mid_backtest.dat"));
    std::unique_ptr<Sender> ordersender(new Sender("order_sub", "connect", "ipc", "order.dat"));
    std::vector<BaseStrategy*> sv;
    // while (true) {
      sv.clear();
      for (int i = 0; i < strategies.getLength(); i++) {
        const libconfig::Setting & param_setting = strategies[i];
        bool no_close_today = false;
        if (param_setting.exists("no_close_today")) {
          no_close_today = param_setting["no_close_today"];
        }
        if (param_setting.exists("strategy_list")) {
          const libconfig::Setting &s_list = param_setting["strategy_list"];
          for (int j = 0; j < s_list.getLength(); j++) {
            std::string con = s_list[j];
            const libconfig::Setting & ticker_setting = ticker_setting_map[ticker_index_map[con]];
            sv.emplace_back(new Strategy(param_setting, ticker_setting, tc, &ticker_strat_map, ct, sender.get(), ordersender.get(), "test", no_close_today));
          }
        } else {
          std::string con = param_setting["unique_name"];
          const libconfig::Setting & ticker_setting = ticker_setting_map[ticker_index_map[con]];
          sv.emplace_back(new Strategy(param_setting, ticker_setting, tc, &ticker_strat_map, ct, sender.get(), ordersender.get(), "test", no_close_today));
        }
      }
      tc.StartTimer();
      for (auto file_name : file_v) {
        printf("This file's tsm: ");
        for (auto a:ticker_strat_map) {
          cout << a.first << " ";
        }
        printf("\n");
        std::string file_mode = Split(file_name, ".").back();
        if (file_mode == "gz") {
          gzFile gzfp = gzopen(file_name.c_str(), "rb");
          if (!gzfp) {
            printf("gzfile open failed!%s\n", file_name.c_str());
            continue;
          }
          printf("handling %s\n", file_name.c_str());
          unsigned char buf[SIZE_OF_SNAPSHOT];
          MarketSnapshot* shot;
          bool is_cut = false;
          while (gzread(gzfp, buf, sizeof(*shot)) > 0) {
            shot = reinterpret_cast<MarketSnapshot*>(buf);
            if (!shot->IsGood()) {
              continue;
            }
            ct.UpdateByShot(*shot);
            if (ticker_strat_map.find(shot->ticker) == ticker_strat_map.end()) {
              continue;
            }
            if ((shot->time.tv_sec+8*3600) % (24*3600) >= 15*3600-10 && !is_cut) {
              for (auto v : sv) {
                v->Clear();
              }
              is_cut = true;
            }
            shot->is_initialized = true;
            std::vector<BaseStrategy*> ticker_sv = ticker_strat_map[shot->ticker];
            for (auto v : ticker_sv) {
              v->UpdateData(*shot);
            }
          }
          gzclose(gzfp);
        } else if (file_mode == "dat") {
          std::ifstream raw_file;
          raw_file.open(file_name.c_str(), ios::in|ios::binary);
          if (!raw_file) {
            printf("%s is not existed!\n", file_name.c_str());
            continue;
          }
          printf("handling %s\n", file_name.c_str());
          MarketSnapshot shot;
          bool is_cut = false;
          while (raw_file.read(reinterpret_cast<char *>(&shot), sizeof(shot))) {
            if (!shot.IsGood()) {
              continue;
            }
            ct.UpdateByShot(shot);
            if (ticker_strat_map.find(shot.ticker) == ticker_strat_map.end()) {
              continue;
            }
            if ((shot.time.tv_sec+8*3600) % (24*3600) >= 15*3600-10 && !is_cut) {
              for (auto v : sv) {
                v->Clear();
              }
              is_cut = true;
            }
            // data_sender->Send(shot.Copy().c_str());
            shot.is_initialized = true;
            std::vector<BaseStrategy*> ticker_sv = ticker_strat_map[shot.ticker];
            for (auto v : ticker_sv) {
              v->UpdateData(shot);
            }
          }
          raw_file.close();
        } else {
          printf("unknown file_mode %s\n", file_name.c_str());
          continue;
        }
        for (auto v : sv) {
          v->Clear();
          v->UpdateCT(ct);
          v->UpdateTicker();
        }
        ct.Clear();
        ct.EnReady();
      }
      tc.EndTimer();
      printf("repeat!!!!\n");
    // }
    order_file.close();
    exchange_file.close();
    strat_file.close();
    for (auto i : sv) {
      delete i;
    }
    printf("backtest over!\n");
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
}
