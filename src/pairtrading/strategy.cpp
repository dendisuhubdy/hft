#include <iostream>
#include <string>
#include <algorithm>
#include <vector>

#include "./strategy.h"

Strategy::Strategy(const libconfig::Setting & param_setting, std::unordered_map<std::string, std::vector<BaseStrategy*> >*ticker_strat_map, ZmqSender<MarketSnapshot>* uisender, ZmqSender<Order>* ordersender, TimeController* tc, ContractWorker* cw, const std::string & date, StrategyMode::Enum mode, std::ofstream* exchange_file)
  : date(date),
    max_close_try(10),
    no_close_today(false),
    max_round(10000),
    close_round(0),
    sample_head(0),
    sample_tail(0),
    exchange_file(exchange_file) {
  m_tc = tc;
  m_cw = cw;
  if (FillStratConfig(param_setting)) {
    RunningSetup(ticker_strat_map, uisender, ordersender);
  }
}

Strategy::~Strategy() {
}

void Strategy::RunningSetup(std::unordered_map<std::string, std::vector<BaseStrategy*> >*ticker_strat_map, ZmqSender<MarketSnapshot>* uisender, ZmqSender<Order>* ordersender) {
  ui_sender = uisender;
  order_sender = ordersender;
  (*ticker_strat_map)[main_ticker].emplace_back(this);
  (*ticker_strat_map)[hedge_ticker].emplace_back(this);
  (*ticker_strat_map)["positionend"].emplace_back(this);
  MarketSnapshot shot;
  shot_map[main_ticker] = shot;
  shot_map[hedge_ticker] = shot;
  avgcost_map[main_ticker] = 0.0;
  avgcost_map[hedge_ticker] = 0.0;
}

bool Strategy::FillStratConfig(const libconfig::Setting& param_setting) {
  try {
    std::string unique_name = param_setting["unique_name"];
    const libconfig::Setting & contract_setting = m_cw->Lookup(unique_name);
    m_strat_name = unique_name;
    auto v = m_cw->GetActiveContracts(unique_name, date);
    if (v.size() < 2) {
      printf("no enough ticker for %s\n", unique_name.c_str());
      PrintVector(v);
      return false;
    }
    main_ticker = v[1];
    hedge_ticker = v[0];
    max_pos = param_setting["max_position"];
    train_samples_ = param_setting["train_samples_s"];
    double m_r = param_setting["min_range"];
    double m_p = param_setting["min_profit"];
    min_price_move = contract_setting["min_price_move"];
    min_profit = m_p * min_price_move;
    min_range = m_r * min_price_move;
    double add_margin = param_setting["add_margin"];
    increment = add_margin*min_price_move;
    double spread_threshold_int = param_setting["spread_threshold"];
    spread_threshold = spread_threshold_int*min_price_move;
    stop_loss_margin = param_setting["stop_loss_margin"];
    max_loss_times = param_setting["max_loss_times"];
    max_holding_sec = param_setting["max_holding_sec"];
    range_width = param_setting["range_width"];
    std::string con = GetCon(main_ticker);
    cancel_limit = contract_setting["cancel_limit"];
    max_round = param_setting["max_round"];
    split_num = param_setting["split_num"];
    if (param_setting.exists("no_close_today")) {
      no_close_today = param_setting["no_close_today"];
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
  return true;
}

void Strategy::Stop() {
  CancelAll(main_ticker);
  ss = StrategyStatus::Stopped;
}

bool Strategy::IsAlign() {
  return shot_map[main_ticker].time.tv_sec == shot_map[hedge_ticker].time.tv_sec && abs(shot_map[main_ticker].time.tv_usec-shot_map[hedge_ticker].time.tv_usec) < 100000;
}


void Strategy::DoOperationAfterCancelled(Order* o) {
  if (cancel_map[o->ticker] > cancel_limit) {
    printf("ticker %s hit cancel limit!\n", o->ticker);
    Stop();
  }
}

double Strategy::OrderPrice(const std::string & ticker, OrderSide::Enum side, bool control_price) {
  return 0.0;
}

void Strategy::CalParams() {
  if (sample_tail < train_samples_) {
    printf("no enough data\n");
    exit(1);
  }
  auto long_params = CalMeanStd(long_, sample_tail - train_samples_, train_samples_);
  auto short_params = CalMeanStd(short_, sample_tail - train_samples_, train_samples_);
  double long_mean = std::get<0>(long_params);
  double long_std = std::get<1>(long_params);
  double short_mean = std::get<0>(short_params);
  double short_std = std::get<1>(short_params);
  FeePoint main_point = m_cw->CalFeePoint(main_ticker, GetMid(main_ticker), 1, GetMid(main_ticker), 1, no_close_today);
  FeePoint hedge_point = m_cw->CalFeePoint(hedge_ticker, GetMid(hedge_ticker), 1, GetMid(hedge_ticker), 1, no_close_today);
  double round_fee_cost = main_point.open_fee_point + main_point.close_fee_point + hedge_point.open_fee_point + hedge_point.close_fee_point;
  double long_width = std::max(range_width * long_std, min_range) + round_fee_cost;
  double short_width = std::max(range_width * short_std, min_range) + round_fee_cost;
  long_up_ = long_mean + long_width;
  short_up_ = short_mean + short_width;
  long_down_ = long_mean - long_width;
  short_down_ = short_mean - short_mean;
  sample_head = sample_tail;
}

void Strategy::ForceFlat() {
  int pos = position_map[main_ticker];
  if (pos == 0) {
    return;
  }
  OrderSide::Enum side = (pos > 0) ? OrderSide::Sell : OrderSide::Buy;
  for (int i = 0; i < max_close_try; i++) {
    if (Close(side)) {
      break;
    }
    if (i == max_close_try - 1) {
      printf("[%s %s]try max_close times, cant close this order!\n", main_ticker.c_str(), hedge_ticker.c_str());
      PrintMap(order_map);
      order_map.clear();  // it's a temp solution, TODO
      Close(side);
    }
  }
}

bool Strategy::Close(OrderSide::Enum side) {
  if (!order_map.empty()) {
    printf("[%s %s]block order exsited! no close\n", main_ticker.c_str(), hedge_ticker.c_str());
    PrintMap(order_map);
    return false;
  }
  double price = (side == OrderSide::Buy) ? shot_map[main_ticker].asks[0] : shot_map[main_ticker].bids[0];
  int64_t size = (side == OrderSide::Buy) ? 1 : -1;
  Order* o = PlaceOrder(main_ticker, price, size, no_close_today, "close");
  o->Show(stdout);
  return true;
}

void Strategy::CloseLogic() {
  int pos = position_map[main_ticker];
  if (pos == 0) {
    return;
  }
  double long_back = long_.back();
  double short_back = short_.back();
  if (pos > 0 && short_back > short_mean_) {  // buy pos, sell to close
    Close(OrderSide::Sell);
  }
  if (pos < 0 && long_back < long_mean_) {
    Close(OrderSide::Buy);
  }
}

void Strategy::Flatting() {
  if (RiskCheck()) {
    CloseLogic();
  }
}

void Strategy::Open(OrderSide::Enum side) {
  if (!order_map.empty()) {
    printf("block order exsited! no open \n");
    PrintMap(order_map);
    return;
  }
  double price = (side == OrderSide::Buy) ? shot_map[main_ticker].asks[0] : shot_map[main_ticker].bids[0];
  int64_t size = (side == OrderSide::Buy) ? 1 : -1;
  Order* o = PlaceOrder(main_ticker, price, size, no_close_today, "open");
  target_hedge_price = (side == OrderSide::Buy) ? shot_map[hedge_ticker].bids[0] : shot_map[hedge_ticker].asks[0];
  o->Show(stdout);
}

bool Strategy::OpenLogic() {
  double long_back = long_.back();
  double short_back = short_.back();
  if (abs(position_map[main_ticker]) >= max_pos || (long_back > short_down_ && short_back < long_up_)) {
  return false;
  }
  OrderSide::Enum side = (long_back <= short_down_) ? OrderSide::Buy : OrderSide::Sell;
  Open(side);
  return true;
}

bool Strategy::RiskCheck() {
  return IsAlign() && close_round < max_round;
}

void Strategy::Run() {
  if (RiskCheck()) {
    if (!OpenLogic()) {
      CloseLogic();
    }
  }
}

void Strategy::Init() {
  ticker_map[main_ticker] = true;
  ticker_map[hedge_ticker] = true;
  ticker_map["positionend"] = true;
}

void Strategy::DoOperationAfterUpdateData(const MarketSnapshot& shot) {
  double long_price = shot_map[main_ticker].asks[0] - shot_map[hedge_ticker].bids[0];
  double short_price = shot_map[main_ticker].bids[0] - shot_map[hedge_ticker].asks[0];
  long_.push_back(long_price);
  short_.push_back(short_price);
  current_spread = shot_map[main_ticker].asks[0] - shot_map[main_ticker].bids[0] + shot_map[hedge_ticker].asks[0] - shot_map[hedge_ticker].bids[0];
}

void Strategy::Resume() {
  sample_head = sample_tail;
}

bool Strategy::Ready() {
  return sample_tail - sample_head >= train_samples_;
}

void Strategy::ModerateOrders(const std::string & ticker) {
  if (mode_ != StrategyMode::Real) {
    return;
  }
  for (auto m:order_map) {
    Order* o = m.second;
    if (!o->Valid()) {
      continue;
    }
    MarketSnapshot shot = shot_map[o->ticker];
    double reasonable_price = (o->side == OrderSide::Buy ? shot.asks[0] : shot.bids[0]);
    bool is_price_move = (fabs(reasonable_price - o->price) >= min_price_move/2);
    if (!is_price_move) {
      continue;
    }
    if (o->ticker == main_ticker) {
      // if ((o->side == OrderSide::Buy && shot_map[hedge_ticker].bids[0] - this->target_hedge_price < -1e-4) ||
          //  (o->side == OrderSide::Sell && shot_map[hedge_ticker].asks[0] - this->target_hedge_price > -1e-4) ) {
      CancelOrder(o);
      //  }
    } else if (o->ticker == hedge_ticker) {
      ModOrder(o);
    } else {
      continue;
    }
  }
}

void Strategy::Start() {
  CalParams();
}

void Strategy::DoOperationAfterUpdatePos(Order* o, const ExchangeInfo& info) {
}

void Strategy::DoOperationAfterFilled(Order* o, const ExchangeInfo& info) {
  std::string tbd = o->tbd;
  bool is_close = (tbd.find("close") == string::npos);
  if (strcmp(info.ticker, main_ticker.c_str()) == 0) {
    double price = (info.side == OrderSide::Buy) ? shot_map[hedge_ticker].bids[0] : shot_map[hedge_ticker].asks[0];
    int64_t size = (info.side == OrderSide::Buy) ? -1 : 1;
    string orderinfo = is_close ? "close" : "open";
    Order* o = PlaceOrder(hedge_ticker, price, size, no_close_today, orderinfo);
    o->Show(stdout);
  } else if (strcmp(info.ticker, hedge_ticker.c_str()) == 0) {
  } else {
  }
}

bool Strategy::Spread_Good() {
  return current_spread <= spread_threshold;
}
