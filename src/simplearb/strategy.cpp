#include <iostream>
#include <string>
#include <algorithm>
#include <vector>

#include "./strategy.h"

Strategy::Strategy(const libconfig::Setting & param_setting, const libconfig::Setting & contract_setting, const TimeController& tc, std::unordered_map<std::string, std::vector<BaseStrategy*> >*ticker_strat_map, Contractor& ct, Sender* sender, const std::string & mode, std::ofstream* order_file, std::ofstream* exchange_file, std::ofstream* strat_file, bool no_close_today)
  : this_tc(tc),
    mode(mode),
    last_valid_mid(0.0),
    stop_loss_times(0),
    max_close_try(10),
    this_order_file(order_file),
    this_exchange_file(exchange_file),
    this_strat_file(strat_file),
    no_close_today(no_close_today) {
  caler = new CALER(contract_setting);
  try {
    std::string unique_name = param_setting["unique_name"];
    m_strat_name = unique_name;
    std::string ticker1 = param_setting["pairs"][0];
    std::string ticker2 = param_setting["pairs"][1];
    std::vector<std::string> v = ct.GetTicker(unique_name);
    PrintVector(v);
    main_ticker = v[1];
    hedge_ticker = v[0];
    max_pos = param_setting["max_position"];
    min_train_sample = param_setting["min_train_samples"];
    double m_r = param_setting["min_range"];
    double m_p = param_setting["min_profit"];
    min_price_move = contract_setting["min_price_move"];
    printf("[%s, %s] mpv is %lf\n", main_ticker.c_str(), hedge_ticker.c_str(), min_price_move);
    min_profit = m_p * min_price_move;
    min_range = m_r * min_price_move;
    // current_spread = 0.0;
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
    printf("[%s %s] try over!\n", main_ticker.c_str(), hedge_ticker.c_str());
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
  ui_sender = sender;
  printf("here reached!\n");
  (*ticker_strat_map)[main_ticker].emplace_back(this);
  (*ticker_strat_map)[hedge_ticker].emplace_back(this);
  (*ticker_strat_map)["positionend"].emplace_back(this);
  MarketSnapshot shot;
  shot_map[main_ticker] = shot;
  shot_map[hedge_ticker] = shot;
  avgcost_map[main_ticker] = 0.0;
  avgcost_map[hedge_ticker] = 0.0;
  if (mode == "test") {
    position_ready = true;
  }
  printf("[%s %s]strategy init success!\n", main_ticker.c_str(), hedge_ticker.c_str());
  up_diff = 0.0;
  down_diff = 0.0;
  stop_loss_up_line = 0.0;
  stop_loss_down_line = 0.0;
}

Strategy::~Strategy() {
  delete caler;
}

void Strategy::Clear() {
  printf("[%s %s] %ld mid_value size is %zu\n", main_ticker.c_str(), hedge_ticker.c_str(), shot_map[hedge_ticker].time.tv_sec, map_vector.size());
  ss = StrategyStatus::Init;
  ClearPositionRecord();
  mid_map.clear();
  map_vector.clear();
  build_position_time = MAX_UNIX_TIME;
  stop_loss_times = 0;
  m_ct.Clear();
}

void Strategy::Stop() {
  CancelAll(main_ticker);
  ss = StrategyStatus::Stopped;
}

bool Strategy::IsAlign() {
  if (shot_map[main_ticker].time.tv_sec == shot_map[hedge_ticker].time.tv_sec && abs(shot_map[main_ticker].time.tv_usec-shot_map[hedge_ticker].time.tv_usec) < 100000) {
    return true;
  }
  return false;
}

OrderSide::Enum Strategy::OpenLogicSide() {
  double mid = GetPairMid();
  // printf("judge open logic side:mid = %lf, up_diff=%lf, down_diff=%lf\n", mid, up_diff, down_diff);
  // shot_map[main_ticker].Show(stdout);
  // shot_map[hedge_ticker].Show(stdout);
  if (mid - current_spread/2 > up_diff) {
    printf("[%s %s]sell condition hit, as diff id %f\n",  main_ticker.c_str(), hedge_ticker.c_str(), mid);
    return OrderSide::Sell;
  } else if (mid + current_spread/2 < down_diff) {
    printf("[%s %s]buy condition hit, as diff id %f\n", main_ticker.c_str(), hedge_ticker.c_str(), mid);
    return OrderSide::Buy;
  } else {
    return OrderSide::Unknown;
  }
}

void Strategy::DoOperationAfterCancelled(Order* o) {
  printf("contract %s cancel num %d!\n", o->contract, cancel_map[o->contract]);
  if (cancel_map[o->contract] > cancel_limit) {
    printf("contract %s hit cancel limit!\n", o->contract);
    Stop();
  }
}

double Strategy::OrderPrice(const std::string & contract, OrderSide::Enum side, bool control_price) {
  if (contract == hedge_ticker) {
    return (side == OrderSide::Buy)?shot_map[hedge_ticker].asks[0]:shot_map[hedge_ticker].bids[0];
  } else if (contract == main_ticker) {
    return (side == OrderSide::Buy)?shot_map[main_ticker].asks[0]:shot_map[main_ticker].bids[0];
  } else {
    printf("error contract %s\n", contract.c_str());
    return -1.0;
  }
}

void Strategy::CalParams() {
  if (map_vector.size() < min_train_sample) {
    printf("[%s %s]no enough mid data! size if %zu\n", main_ticker.c_str(), hedge_ticker.c_str(), map_vector.size());
    exit(1);
  }
  double avg = 0.0;
  double std = 0.0;
  int cal_head = map_vector.size() - min_train_sample;
  for (unsigned int i = cal_head; i < map_vector.size(); i++) {
    avg += map_vector[i];
  }
  avg /= min_train_sample;
  for (unsigned int i = cal_head; i < map_vector.size(); i++) {
    std += (map_vector[i]-avg) * (map_vector[i]-avg);
  }
  std /= min_train_sample;
  std = sqrt(std);
  FeePoint main_point = caler->CalFeePoint(main_ticker, GetMid(main_ticker), 1, GetMid(main_ticker), 1, no_close_today);
  FeePoint hedge_point = caler->CalFeePoint(hedge_ticker, GetMid(hedge_ticker), 1, GetMid(hedge_ticker), 1, no_close_today);
  round_fee_cost = main_point.open_fee_point + main_point.close_fee_point + hedge_point.open_fee_point + hedge_point.close_fee_point;
  double margin = std::max(range_width * std, min_range) + round_fee_cost;
  up_diff = avg + margin;
  down_diff = avg - margin;
  stop_loss_up_line = up_diff + stop_loss_margin * margin;
  stop_loss_down_line = down_diff - stop_loss_margin * margin;
  // down_diff = std::min(avg - range_width * std, avg-min_profit);
  mean = avg;
  spread_threshold = margin - min_profit - round_fee_cost;
  printf("[%s %s]cal done,mean is %lf, std is %lf, parmeters: [%lf,%lf], spread_threshold is %lf, min_profit is %lf, up_loss=%lf, down_loss=%lf fee_point=%lf\n", main_ticker.c_str(), hedge_ticker.c_str(), avg, std, down_diff, up_diff, spread_threshold, min_profit, stop_loss_up_line, stop_loss_down_line, round_fee_cost);
}

bool Strategy::HitMean() {
  double this_mid = GetPairMid();
  int pos = position_map[main_ticker];
  if (pos > 0 && this_mid - current_spread/2 >= mean) {  // buy position
    printf("[%s %s] mean is %lf, this_mid is %lf, current_spread is %lf, pos is %d\n", main_ticker.c_str(), hedge_ticker.c_str(), mean, this_mid, current_spread, pos);
    return true;
  } else if (pos < 0 && this_mid + current_spread/2 <= mean) {  // sell position
    printf("[%s %s] mean is %lf, this_mid is %lf, current_spread is %lf, pos is %d\n", main_ticker.c_str(), hedge_ticker.c_str(), mean, this_mid, current_spread, pos);
    return true;
  }
  return false;
}

void Strategy::ForceFlat() {
  printf("%ld [%s %s]this round hit stop_loss condition, pos:%d current_mid:%lf, current_spread:%lf stoplossline %lf-%lf forceflat\n", shot_map[hedge_ticker].time.tv_sec, main_ticker.c_str(), hedge_ticker.c_str(), position_map[main_ticker], GetPairMid(), current_spread, stop_loss_down_line, stop_loss_up_line);
  shot_map[main_ticker].Show(stdout);
  shot_map[hedge_ticker].Show(stdout);
  for (int i = 0; i < max_close_try; i++) {
    if (Close(true)) {
      break;
    }
    if (i == max_close_try - 1) {
      printf("[%s %s]try max_close times, cant close this order!\n", main_ticker.c_str(), hedge_ticker.c_str());
      PrintMap(order_map);
      order_map.clear();  // it's a temp solution, TODO
      Close();
    }
  }
  CalParams();
}

bool Strategy::Close(bool force_flat) {
  int pos = position_map[main_ticker];
  if (pos == 0) {
    return true;
  }
  OrderSide::Enum pos_side = pos > 0 ? OrderSide::Buy: OrderSide::Sell;
  OrderSide::Enum close_side = pos > 0 ? OrderSide::Sell: OrderSide::Buy;
  double hedge_price = pos > 0 ? shot_map[hedge_ticker].asks[0] : shot_map[hedge_ticker].bids[0];
  printf("close using %s: pos is %d, diff is %lf\n", OrderSide::ToString(close_side), pos, GetPairMid());
  // printf("spread is %lf %lf min_profit is %lf\n", shot_map[main_ticker].asks[0]-shot_map[main_ticker].bids[0], shot_map[hedge_ticker].asks[0]-shot_map[hedge_ticker].bids[0], min_profit);
  if (order_map.empty()) {
    PrintMap(avgcost_map);
    Order* o = NewOrder(main_ticker, close_side, abs(pos), false, false, force_flat ? "force_flat" : "close", no_close_today);  // close
    o->Show(stdout);
    HandleTestOrder(o);
    if (mode == "real") {
      double this_round_pnl = caler->CalNetPnl(main_ticker, avgcost_map[main_ticker], abs(pos), o->price, abs(pos), close_side, no_close_today) + caler->CalNetPnl(hedge_ticker, avgcost_map[hedge_ticker], abs(pos), hedge_price, abs(pos), pos_side, no_close_today);
      Fee main_fee = caler->CalFee(main_ticker, avgcost_map[main_ticker], abs(pos), shot_map[main_ticker].  bids[0], abs(pos), no_close_today);
      Fee hedge_fee = caler->CalFee(hedge_ticker, avgcost_map[hedge_ticker], abs(pos), hedge_price, abs(pos), no_close_today);
      double this_round_fee = main_fee.open_fee + main_fee.close_fee + hedge_fee.open_fee + hedge_fee.close_fee;
      printf("%ld [%s %s]%sThis round close pnl: %lf, fee_cost: %lf pos is %d, holding second is %ld\n", shot_map[hedge_ticker].time.tv_sec, main_ticker.c_str(), hedge_ticker.c_str(), force_flat ? "[Time up] " : "", this_round_pnl, this_round_fee, pos, shot_map[hedge_ticker].time.tv_sec - build_position_time);
    }
    return true;
  } else {
    printf("[%s %s]block order exsited! no close\n", main_ticker.c_str(), hedge_ticker.c_str());
    PrintMap(order_map);
    // exit(1);
    return false;
  }
}

double Strategy::GetPairMid() {
  return GetMid(main_ticker) - GetMid(hedge_ticker);
}

void Strategy::StopLossLogic() {
  if (!Spread_Good()) {
    return;
  }
  int pos = position_map[main_ticker];
  double this_mid = GetPairMid();
  if (pos > 0) {  // buy position
    if (this_mid < stop_loss_down_line) {  // stop condition meets
      ForceFlat();
      stop_loss_times += 1;
    }
  } else if (pos < 0) {  // sell position
    if (this_mid > stop_loss_up_line) {  // stop condition meets
      ForceFlat();
      stop_loss_times += 1;
    }
  }
  if (stop_loss_times >= max_loss_times) {
    ss = StrategyStatus::Stopped;
    printf("stop loss times hit max!\n");
  }
}

void Strategy::CloseLogic() {
  StopLossLogic();
  int pos = position_map[main_ticker];
  if (pos == 0) {
    return;
  }

  if (TimeUp()) {
    printf("[%s %s] holding time up, start from %ld, now is %ld, max_hold is %d close diff is %lf force to close position!\n", main_ticker.c_str(), hedge_ticker.c_str(), build_position_time, mode == "test" ? shot_map[main_ticker].time.tv_sec : m_tc->GetSec(), max_holding_sec, GetPairMid());
    ForceFlat();
    return;
  }

  if (HitMean()) {
    if (Close()) {
      CalParams();
    }
    return;
  }
}

void Strategy::Flatting() {
  if (!IsAlign()) {
    CloseLogic();
  }
}

void Strategy::Open(OrderSide::Enum side) {
  int pos = position_map[main_ticker];
  printf("[%s %s] open %s: pos is %d, diff is %lf\n", main_ticker.c_str(), hedge_ticker.c_str(), OrderSide::ToString(side), pos, GetPairMid());
  if (order_map.empty()) {  // no block order, can add open
    Order* o = NewOrder(main_ticker, side, 1, false, false, "", no_close_today);
    o->Show(stdout);
    // printf("spread is %lf %lf min_profit is %lf, next open will be %lf\n", shot_map[main_ticker].asks[0]-shot_map[main_ticker].bids[0], shot_map[hedge_ticker].asks[0]-shot_map[hedge_ticker].bids[0], min_profit, side == OrderSide::Buy ? down_diff: up_diff);
    HandleTestOrder(o);
  } else {  // block order exsit, no open, possible reason: no enough margin
    printf("block order exsited! no open \n");
    PrintMap(order_map);
    // exit(1);
  }
}

bool Strategy::OpenLogic() {
  OrderSide::Enum side = OpenLogicSide();
  if (side == OrderSide::Unknown) {
    return false;
  }
  // do meet the logic
  int pos = position_map[main_ticker];
  if (abs(pos) == max_pos) {
    // hit max, still update bound
    // UpdateBound(side == OrderSide::Buy ? OrderSide::Sell : OrderSide::Buy);
    return false;
  }
  Open(side);
  return true;
}

void Strategy::Run() {
  if (IsAlign()) {
      if (!OpenLogic()) {
        CloseLogic();
      }
  } else {
  }
}

/*
void Strategy::InitTicker() {
  ticker_map[main_ticker] = true;
  ticker_map[hedge_ticker] = true;
  ticker_map["positionend"] = true;
}

void Strategy::InitTimer() {
  m_tc = &this_tc;
}

void Strategy::InitFile() {
  order_file = this_order_file;
  exchange_file = this_exchange_file;
}
*/

void Strategy::Init() {
  m_tc = &this_tc;
  order_file = this_order_file;
  exchange_file = this_exchange_file;
  strat_file = this_strat_file;
  // ui_file = this_ui_file;
  ticker_map[main_ticker] = true;
  ticker_map[hedge_ticker] = true;
  ticker_map["positionend"] = true;
}

void Strategy::DoOperationAfterUpdateData(const MarketSnapshot& shot) {
  mid_map[shot.ticker] = (shot.bids[0]+shot.asks[0]) / 2;  // mid_map saved the newest mid, no matter it is aligned or not
  current_spread = shot_map[main_ticker].asks[0] - shot_map[main_ticker].bids[0] + shot_map[hedge_ticker].asks[0] - shot_map[hedge_ticker].bids[0];
  if (IsAlign()) {
    double mid = GetPairMid();
    if (mode != "test") {
      printf("%ld [%s, %s]mid_diff is %lf\n", shot.time.tv_sec, main_ticker.c_str(), hedge_ticker.c_str(), mid_map[main_ticker]-mid_map[hedge_ticker]);
    }
    MarketSnapshot shot;
    snprintf(shot.ticker, sizeof(shot.ticker), "['%s', '%s']", main_ticker.c_str(), hedge_ticker.c_str());
    shot.time = shot_map[hedge_ticker].time;
    shot.bids[0] = down_diff - current_spread/2;
    shot.bids[1] = stop_loss_down_line;
    shot.bids[2] = mean - current_spread/2;
    shot.asks[0] = up_diff + current_spread/2;
    shot.asks[1] = stop_loss_up_line;
    shot.asks[2] = mean + current_spread/2;
    shot.open_interest = mean;
    std::string label = main_ticker + '|' + hedge_ticker;
    snprintf(shot.ticker, sizeof(shot.ticker), "%s", label.c_str());
    shot.last_trade = mid;
    if (this_strat_file) {
      shot.Show(*this_strat_file);
    }
    map_vector.emplace_back(mid);  // map_vector saved the aligned mid, all the elements here are safe to trade
    ui_sender->Send(shot);
  }
}

void Strategy::HandleCommand(const MarketSnapshot& shot) {
  printf("received command!\n");
  shot.Show(stdout);
  if (abs(shot.asks[0]) > MIN_DOUBLE_DIFF) {
    up_diff = shot.asks[0];
    return;
  }
  if (abs(shot.bids[0]) > MIN_DOUBLE_DIFF) {
    down_diff = shot.bids[0];
    return;
  }
  if (abs(shot.asks[1]) > MIN_DOUBLE_DIFF) {
    stop_loss_up_line = shot.asks[1];
    return;
  }
  if (abs(shot.bids[1]) > MIN_DOUBLE_DIFF) {
    stop_loss_down_line = shot.bids[1];
    return;
  }
}

void Strategy::Train() {
}

void Strategy::Pause() {
}

void Strategy::Resume() {
  Run();
}

bool Strategy::Ready() {
  if (position_ready && shot_map[main_ticker].IsGood() && shot_map[hedge_ticker].IsGood() && map_vector.size() >= min_train_sample) {
    if (map_vector.size() == min_train_sample) {
      // first cal params
      CalParams();
    }
    return true;
  }
  if (!position_ready) {
    printf("waiting position query finish!\n");
  }
  return false;
}

void Strategy::ModerateOrders(const std::string & contract) {
  // just make sure the order filled
  if (mode == "real") {
    for (auto m:order_map) {
      Order* o = m.second;
      if (o->Valid()) {
        std::string ticker = o->contract;
        MarketSnapshot shot = shot_map[ticker];
        double reasonable_price = (o->side == OrderSide::Buy ? shot.asks[0] : shot.bids[0]);
        bool is_price_move = (fabs(reasonable_price - o->price) >= min_price_move);
        if (!is_price_move) {
          continue;
        }
        if (ticker == main_ticker) {
          CancelOrder(o);
          printf("[%s %s]Abandon this oppounity because main ticker price change %lf->%lf mpv=%lf\n", main_ticker.c_str(), hedge_ticker.c_str(), o->price, reasonable_price, min_price_move);
        } else if (ticker == hedge_ticker) {
          ModOrder(o);
          printf("[%s %s]Slip point for :modify %s order %s: %lf->%lf mpv=%lf\n", main_ticker.c_str(), hedge_ticker.c_str(), OrderSide::ToString(o->side), o->order_ref, o->price, reasonable_price, min_price_move);
        } else {
          continue;
        }
      }
    }
  }
}

void Strategy::ClearPositionRecord() {
  avgcost_map.clear();
  position_map.clear();
}

void Strategy::Start() {
  if (!is_started) {
    ClearPositionRecord();
    is_started = true;
  }
  Run();
}

void Strategy::DoOperationAfterUpdatePos(Order* o, const ExchangeInfo& info) {
}

void Strategy::UpdateBound(OrderSide::Enum side) {
  printf("Entering UpdateBound\n");
  int pos = position_map[main_ticker];
  if (pos == 0) {  // close operation filled, no update bound
    return;
  }
  if (side == OrderSide::Sell) {
    down_diff = GetPairMid();
    down_diff -= increment;
    if (abs(pos) > 1) {
      mean -= increment/2;
      stop_loss_down_line -= increment/2;
    }
  } else {
    up_diff = GetPairMid();
    up_diff += increment;
    if (abs(pos) > 1) {
      mean += increment/2;
      stop_loss_up_line += increment/2;
    }
  }
  printf("spread is %lf %lf min_profit is %lf, next open will be %lf mean is %lf\n", shot_map[main_ticker].asks[0]-shot_map[main_ticker].bids[0], shot_map[hedge_ticker].asks[0]-shot_map[hedge_ticker].bids[0], min_profit, side == OrderSide::Sell ? down_diff: up_diff, mean);
}

void Strategy::HandleTestOrder(Order* o) {
  if (mode != "test") {
    return;
  }
  ExchangeInfo info;
  info.trade_size = o->size;
  position_map[o->contract] += o->side == OrderSide::Buy ? o->size : -o->size;
  order_map.clear();
  DoOperationAfterFilled(o, info);
}


void Strategy::UpdateBuildPosTime() {
  int hedge_pos = position_map[hedge_ticker];
  if (hedge_pos == 0) {  // closed all position, reinitialize build_position_time
    build_position_time = MAX_UNIX_TIME;
  } else if (hedge_pos == 1) {  // position 0->1, record build_time
    build_position_time = m_tc->GetStratSec(last_shot.time);
  }
}

void Strategy::DoOperationAfterFilled(Order* o, const ExchangeInfo& info) {
  if (strcmp(o->contract, main_ticker.c_str()) == 0) {
    // get hedged right now
    Order* order = NewOrder(hedge_ticker, (o->side == OrderSide::Buy)?OrderSide::Sell : OrderSide::Buy, info.trade_size, false, false, "", no_close_today);
    order->Show(stdout);
    HandleTestOrder(order);
  } else if (strcmp(o->contract, hedge_ticker.c_str()) == 0) {
    UpdateBuildPosTime();
    UpdateBound(o->side);
  } else {
    SimpleHandle(322);
  }
}

bool Strategy::Spread_Good() {
  return (current_spread > spread_threshold) ? false : true;
}
