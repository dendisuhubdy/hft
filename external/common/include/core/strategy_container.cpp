#include "strategy_container.h"

StrategyContainer::StrategyContainer(unordered_map<string, vector<BaseStrategy*> > &m)
  : m(m),
    marketdata_recver(new Recver("data_pub")),
    exchangeinfo_recver(new Recver("exchange_info")),
    command_recver(new Recver("*:33334", "tcp", "bind")) {

}

StrategyContainer::~StrategyContainer() {
}

void StrategyContainer::RunExchangeListener(unordered_map<std::string, std::vector<BaseStrategy*> >& m, Recver* exchangeinfo_recver) {
  while (true) {
    ExchangeInfo info;
    exchangeinfo_recver->Recv(info);
    info.Show(stdout);
    std::vector<BaseStrategy*> sv = m[info.ticker];
    for (auto v : sv) {
      v->UpdateExchangeInfo(info);
    }
  }
}

void StrategyContainer::RunCommandListener(unordered_map<string, vector<BaseStrategy*> > &m, Recver* command_recver) {
  while (true) {
    Command shot;
    command_recver->Recv(shot);
    printf("command recved!\n");
    shot.Show(stdout);
    string ticker = Split(shot.ticker, "|").front();
    if (ticker == "load_history") {
      // Load_history("mid.dat");
      continue;
    }
    vector<BaseStrategy*> sv = m[ticker];
    for (auto v : sv) {
      v->HandleCommand(shot);
    }
  }
}

void StrategyContainer::RunMarketDataListener(unordered_map<string, vector<BaseStrategy*> > &m, Recver* marketdata_recver) {
  while (true) {
    MarketSnapshot shot;
    marketdata_recver->Recv(shot);
    auto sv = m[shot.ticker];
    for (auto s : sv) {
      s->UpdateData(shot);
    }
  }
}

void StrategyContainer::Start() {
  thread command_thread(RunCommandListener, std::ref(m), command_recver.get());
  thread exchangeinfo_thread(RunExchangeListener, std::ref(m), exchangeinfo_recver.get());
  thread marketdata_thread(RunMarketDataListener, std::ref(m), marketdata_recver.get());
  command_thread.join();
  exchangeinfo_thread.join();
  marketdata_thread.join();
}
