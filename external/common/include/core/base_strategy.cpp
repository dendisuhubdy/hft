#include "base_strategy.h"

BaseStrategy::BaseStrategy()
  : position_ready(false),
    ref_num(0),
    m_tc("/root/hft/config/prod/time.config"),
    ss(StrategyStatus::Init),
    build_position_time(MAX_UNIX_TIME),
    max_holding_sec(36000),
    m_ct("/root/hft/config/contract/bk_contract.config"),
    m_cal(m_ct),
    init_ticker(false) {
  cout << "base no-param constructor called!" << endl;
}

/*
BaseStrategy::BaseStrategy(const std::string& contract_config_path)
  : position_ready(false),
    ref_num(0),
    ss(StrategyStatus::Init),
    build_position_time(MAX_UNIX_TIME),
    max_holding_sec(36000),
    dt(Dater()),
    m_hw(dt.GetValidFile(dt.GetCurrentDate(), -10)) ,
    init_ticker(false) {
  contract_config.readFile(contract_config_path.c_str());
  cout << "base one-param constructor called!" << endl;
  pthread_mutex_init(&cancel_mutex, nullptr);
  pthread_mutex_init(&order_ref_mutex, nullptr);
  pthread_mutex_init(&mod_mutex, nullptr);
}
*/

void BaseStrategy::HandleCommand(const Command& shot) {
}

BaseStrategy::~BaseStrategy() {
  /*if (ui_sender) {
    delete ui_sender;
  }*/
}

double BaseStrategy::GetMid(const std::string & ticker) {
  return (shot_map[ticker].bids[0]+shot_map[ticker].asks[0])/2;
}

bool BaseStrategy::TimeUp() const {
  int strat_sec = m_tc.TimevalInt(last_shot.time);
  long int holding_sec = strat_sec - build_position_time;
  if (holding_sec > max_holding_sec) {
    return true;
  }
  return false;
}

bool BaseStrategy::PriceChange(double current_price, double reasonable_price, OrderSide::Enum side, double edurance) {
  if (abs(current_price - reasonable_price) < edurance) {
    return false;
  }
  return true;
}

void BaseStrategy::Clear() {
}

void BaseStrategy::ModerateOrders(const std::string & ticker) {
}


void BaseStrategy::UpdateAvgCost(const std::string & ticker, double trade_price, int size) {
  double capital_change = trade_price*size;
  int current_pos = position_map[ticker];
  int pre_pos = current_pos - size;
  avgcost_map[ticker] = (avgcost_map[ticker] * pre_pos + capital_change)/current_pos;
}

std::string BaseStrategy::GenOrderRef() {
  std::lock_guard<std::mutex> lg(order_ref_mutex);
  char orderref[32];
  snprintf(orderref, sizeof(orderref), "%s%d", m_strat_name.c_str(), ref_num++);
  return orderref;
}

Order* BaseStrategy::NewOrder(const std::string & ticker, OrderSide::Enum side, int size, bool control_price, bool sleep_order, const std::string & tbd, bool no_today) {
    if (size == 0) {
      return nullptr;
    }
    Order* order = new Order();
    order->shot_time.tv_sec = shot_map[ticker].time.tv_sec;
    order->shot_time.tv_usec = shot_map[ticker].time.tv_usec;
    gettimeofday(&order->send_time, nullptr);
    order->traded_size = 0;
    snprintf(order->ticker, sizeof(order->ticker), "%s", ticker.c_str());
    order->price = OrderPrice(ticker, side, control_price);
    order->size = size;
    order->side = side;
    if (no_today) {
      order->offset = Offset::NO_TODAY;
    }
    snprintf(order->order_ref, sizeof(order->order_ref), "%s", GenOrderRef().c_str());
    order->action = OrderAction::NewOrder;
    order->status = OrderStatus::SubmitNew;
    if (tbd != "") {
      snprintf(order->tbd, sizeof(order->tbd), "%s", tbd.c_str());
    }
    if (sleep_order) {
      order->status = OrderStatus::Sleep;
      printf("order %s is a sleep order\n", order->order_ref);
    }
    order_map[order->order_ref] = order;
    if (!sleep_order) {
      order_sender->Send(*order);
    }
    return order;
}

void BaseStrategy::ModOrder(Order* o, bool sleep) {
  printf("modorder lock\n");
  std::lock_guard<std::mutex> lg(mod_mutex);
  o->shot_time = shot_map[o->ticker].time;
  gettimeofday(&o->send_time, nullptr);
  o->price = OrderPrice(o->ticker, o->side, false);
  o->status = OrderStatus::Modifying;
  o->action = OrderAction::ModOrder;
  if (sleep) {
    o->status = OrderStatus::Sleep;
  }
  printf("release modorder lock\n");
  order_sender->Send(*o);
}

void BaseStrategy::Wakeup() {
  for (std::unordered_map<std::string, Order*>::iterator it = order_map.begin(); it != order_map.end(); it++) {
    Order* o = it->second;
    if (o->status == OrderStatus::Sleep) {
      char wakestr[1024];
      snprintf(wakestr, sizeof(wakestr), "%s%s", "wake", o->tbd);
      NewOrder(o->ticker, o->side, o->size, false, false, wakestr, o->offset == Offset::NO_TODAY);
      DelOrder(o->order_ref);
    }
  }
}

void BaseStrategy::Wakeup(Order* o) {
  char wakestr[1024];
  snprintf(wakestr, sizeof(wakestr), "%s%s", "wake", o->tbd);
  NewOrder(o->ticker, o->side, o->size, false, false, wakestr, o->offset == Offset::NO_TODAY);
  DelOrder(o->order_ref);
}

void BaseStrategy::RequestQryPos() {
  position_map.clear();
  std::unique_ptr<Order> op(new Order());
  Order* o = op.get();
  o->shot_time = shot_map[o->ticker].time;
  gettimeofday(&o->send_time, nullptr);
  o->action = OrderAction::QueryPos;
  order_sender->Send(*o);
  cout << "request query position sent!" << endl;
}

void BaseStrategy::CancelAll(const std::string & ticker) {
  printf("Enter Cancel ALL for %s\n", ticker.c_str());
  std::lock_guard<std::mutex> lg(cancel_mutex);
  for (std::unordered_map<std::string, Order*>::iterator it = order_map.begin(); it != order_map.end(); it++) {
    if (!strcmp(it->second->ticker, ticker.c_str())) {
      Order* o = it->second;
      if (o->Valid()) {
        o->action = OrderAction::CancelOrder;
        o->status = OrderStatus::Cancelling;
        snprintf(o->order_ref, sizeof(o->order_ref), "%s", it->first.c_str());
        order_sender->Send(*o);
      } else if (o->status == OrderStatus::Modifying) {
        o->action = OrderAction::CancelOrder;
        o->status = OrderStatus::Cancelling;
      } else if (o->status == OrderStatus::Sleep) {
        DelOrder(o->order_ref);
      }
    }
  }
}

void BaseStrategy::CancelAll() {
  std::lock_guard<std::mutex> lg(cancel_mutex);
  for (std::unordered_map<std::string, Order*>::iterator it = order_map.begin(); it != order_map.end(); it++) {
    Order* o = it->second;
    if (o->Valid()) {
      o->action = OrderAction::CancelOrder;
      o->status = OrderStatus::Cancelling;
      snprintf(o->order_ref, sizeof(o->order_ref), "%s", it->first.c_str());
      order_sender->Send(*o);
    } else if (o->status == OrderStatus::Modifying) {
      o->action = OrderAction::CancelOrder;
      o->status = OrderStatus::Cancelling;
    } else if (o->status == OrderStatus::Sleep) {
      DelOrder(o->order_ref);
    }
  }
}

void BaseStrategy::CancelOrder(Order* o) {
  std::lock_guard<std::mutex> lg(cancel_mutex);
  o->action = OrderAction::CancelOrder;
  o->status = OrderStatus::Cancelling;
  order_sender->Send(*o);
}

void BaseStrategy::DelOrder(const std::string & ref) {
  std::unordered_map<std::string, Order*>::iterator it = order_map.find(ref);
  if (it != order_map.end()) {
    delete it->second;
    order_map.erase(it);
  } else {
    printf("Delorder Error! %s not found\n", ref.c_str());
    exit(1);
  }
}

void BaseStrategy::DelSleepOrder(const std::string & ref) {
  std::unordered_map<std::string, Order*>::iterator it = sleep_order_map.find(ref);
  if (it != sleep_order_map.end()) {
    sleep_order_map.erase(it);
  } else {
    printf("DelSleeporder Error! %s not found\n", ref.c_str());
    exit(1);
  }
}

void BaseStrategy::UpdatePos(Order* o, const ExchangeInfo& info) {
  // clock_t start = clock();
  o->traded_size += info.trade_size;
  if (o->size == o->traded_size) {
    o->status = OrderStatus::Filled;
    DelOrder(info.order_ref);
  } else {
    o->status = OrderStatus::Pfilled;
  }
  std::string ticker = o->ticker;
  int trade_size = (o->side == OrderSide::Buy)?info.trade_size:-info.trade_size;
  position_map[ticker] += trade_size;
  double trade_price = info.trade_price;
  bool is_close = (position_map[ticker]*trade_size <= 0);
  if (!is_close) {  // only update avgcost when open traded
    UpdateAvgCost(ticker, trade_price, trade_size);
  }
  DoOperationAfterUpdatePos(o, info);
  // printf("updatePos cost %lf second\n", (static_cast<double>(clock()) - start)); //  / CLOCKS_PER_SEC);
}

void BaseStrategy::UpdateData(const MarketSnapshot& shot) {
  if (!init_ticker) {
    printf("recevied first snapshot of %s, strategy init!\n", shot.ticker);
    Init();
    sleep(1);
    RequestQryPos();
    init_ticker = true;
  }
  if (!shot.IsGood()) {
    return;
  }
  shot_map[shot.ticker] = shot;

  std::string shot_ticker = shot.ticker;
  if (!ticker_map[shot_ticker]) {  // filter those nouse ticker data
    printf("received useless data!\n");
    shot.Show(stdout);
    return;
  }
  last_shot = shot;
  DoOperationAfterUpdateData(shot);
  CheckStatus(shot);
  ModerateOrders(shot_ticker);
}

void BaseStrategy::UpdateExchangeInfo(const ExchangeInfo& info) {
  InfoType::Enum t = info.type;

  if (t == InfoType::Position) {
    if (position_ready) {  // ignore positioninfo after ready
      return;
    }
    if ((info.trade_price < 0.00001 || abs(info.trade_size) == 0) && strcmp(info.ticker, "positionend") != 0) {
      return;
    }
    if (strcmp(info.ticker, "positionend") == 0) {
      printf("position recv finished!\n");
      PrintMap(position_map);
      PrintMap(avgcost_map);
      position_ready = true;
      return;
    } else {
      printf("pre_pos=%d, size=%d, price=%lf, pre_avg=%lf\n", position_map[info.ticker], info.trade_size, info.trade_price, avgcost_map[info.ticker]);
      if (position_map[info.ticker] + info.trade_size == 0) {
        avgcost_map[info.ticker] = 0.0;
      } else {
        avgcost_map[info.ticker] = (info.trade_price/ticker_size*info.trade_size + avgcost_map[info.ticker]*position_map[info.ticker])/(position_map[info.ticker] + info.trade_size);
      }
    }
    position_map[info.ticker] += info.trade_size;
    return;
  }
  std::unordered_map<std::string, Order*>::iterator it = order_map.find(info.order_ref);
  if (it == order_map.end()) {  // not main
      printf("strategy line 191:unknown orderref!%s\n", info.order_ref);
      PrintMap(order_map);
      return;
  }
  Order* order = it->second;

  switch (t) {
    case InfoType::Acc:
    {
      if (order->status == OrderStatus::SubmitNew) {
        order->status = OrderStatus::New;
      } else {
        // TODO(nick): ignore other state?
        return;
      }
    }
      // filter the mess order of info arrived
      break;
    case InfoType::Rej:
    {
      printf("order %s rejected!\n", order->order_ref);
      order->status = OrderStatus::Rejected;
      DelOrder(info.order_ref);
      PrintMap(order_map);
    }
      break;
    case InfoType::Cancelled:
    {
      bool is_sleep = (order->status == OrderStatus::Sleep);
      order->status = OrderStatus::Cancelled;
      cancel_map[order->ticker] += 1;
      
      if (order->action == OrderAction::ModOrder) {
        // printf("order %s status is %s\n", order->order_ref, OrderStatus::ToString(order->status));
        // printf("order size is %d tradedsize is %d\n", order->size, order->traded_size);
        NewOrder(order->ticker, order->side, order->size-order->traded_size, false, is_sleep, order->tbd, order->offset == Offset::NO_TODAY);
      }
      DelOrder(info.order_ref);
      DoOperationAfterCancelled(order);
    }
      break;
    case InfoType::CancelRej:
    {
      if (order->status == OrderStatus::Filled) {
        printf("cancelrej bc filled!%s\n", order->order_ref);
        return;
      }
      order->status = OrderStatus::CancelRej;
      printf("cancel rej for order %s\n", info.order_ref);
      return;
      // TODO(nick):
      // case: cancel filled: ignore
      // case: not permitted in this time, wait to cancel
      // other reason: make up for the cancel failed
    }
      break;
    case InfoType::Filled:
    {
      // DoOperationAfterFilled(order, info);
      UpdatePos(order, info);
      // clock_t start = clock();
      DoOperationAfterFilled(order, info);
      // printf("doopafterfilled cost %lf second\n", (static_cast<double>(clock()) - start));
    }
      break;
    case InfoType::Pfilled:
      // TODO(nick): need to realize
      break;
    default:
      // TODO(nick): handle unknown info
      SimpleHandle(331);
      break;
  }
}

void BaseStrategy::Train() {
}

void BaseStrategy::CheckStatus(const MarketSnapshot& shot) {
  switch (ss) {
    case StrategyStatus::Init:
      if (m_tc.ShotStatus(shot) == TimeStatus::Valid) {
        if (Ready()) {
          ss = StrategyStatus::Running;
          Start();
          return;
        } else {
          ss = StrategyStatus::Training;
          return;
        }
      }
      break;

    case StrategyStatus::Running:
      Run();
      if (m_tc.ShotStatus(shot) == TimeStatus::Close) {
        ss = StrategyStatus::Flatting;
        return;
      } else if (m_tc.ShotStatus(shot) == TimeStatus::Pause) {
        ss = StrategyStatus::Pause;
        return;
      } else if (m_tc.ShotStatus(shot) == TimeStatus::Valid) {
        return;
      } else if (m_tc.ShotStatus(shot) == TimeStatus::ForceClose) {
        ss = StrategyStatus::ForceFlat;
        return;
      } else {
        // do something?
      }
      break;

    case StrategyStatus::Training:
      Train();
      if (Ready()) {
        ss = StrategyStatus::Running;
        Start();
      }
      return;
      break;

    case StrategyStatus::Pause:
      Pause();
      if (m_tc.ShotStatus(shot) == TimeStatus::Valid) {
        ss = StrategyStatus::Running;
        Resume();
        return;
      }
      break;

    case StrategyStatus::Stopped:
      return;
      break;

    case StrategyStatus::Flatting:
      Flatting();
      if (m_tc.ShotStatus(shot) == TimeStatus::ForceClose) {
        ss = StrategyStatus::ForceFlat;
        return;
      }
      /*
      else if (m_tc.ShotStatus(shot) == TimeStatus::Valid) {
        ss = StrategyStatus::Init;
        return;
      }
      */
      break;

    case StrategyStatus::ForceFlat:
      ForceFlat();
      ss = StrategyStatus::Stopped;
      return;
      break;
  }
  return;
}

void BaseStrategy::Flatting() {
}

void BaseStrategy::ForceFlat() {
}

void BaseStrategy::DoOperationAfterCancelled(Order* o) {
}
void BaseStrategy::DoOperationAfterUpdatePos(Order* o, const ExchangeInfo& info) {
}
void BaseStrategy::DoOperationAfterFilled(Order* o, const ExchangeInfo& info) {
}
void BaseStrategy::Print() const {
  printf("original basestrategy print called!\n");
}

void BaseStrategy::DoOperationAfterUpdateData(const MarketSnapshot& shot) {
}

void BaseStrategy::SendPlainText(const std::string & flag, const std::string & s) {
  Order order;
  order.action = OrderAction::PlainText;
  order.shot_time = shot_map[order.ticker].time;
  gettimeofday(&order.send_time, nullptr);
  snprintf(order.tbd, sizeof(order.tbd), "%s", s.c_str());
  snprintf(order.ticker, sizeof(order.ticker), "%s", flag.c_str());
  order_sender->Send(order);
}

void BaseStrategy::debug() const {
  printf("print debug for basestrategy!\n");
}

/*
void BaseStrategy::UpdateCT(const HistoryWorker& ct) {
  // m_hw = ct;
}
*/

void BaseStrategy::UpdateTicker() {
}
