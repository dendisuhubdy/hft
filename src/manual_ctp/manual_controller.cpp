#include "./manual_controller.h"
#include <string>

ManualController::ManualController()
  : order_sender(new Sender<Order> ("order_recver")),
    exchange_recver(new Recver<ExchangeInfo>("exchange_info")) {
}

ManualController::~ManualController() {
}

void ManualController::Start() {
  std::thread send_thread(RunSend, order_sender.get());
  std::thread recv_thread(RunRecv, exchange_recver.get());
  send_thread.join();
  recv_thread.join();
}

void ManualController::RunSend(Sender<Order> * s) {
  int count = 0;
  std::string ticker;
  double price;
  int size;
  OrderSide::Enum side;
  int side_int;
  while (ticker != "quit") {
    std::cout << "ticker:";
    std::cin >> ticker;
    std::cout << "price:";
    std::cin >> price;
    std::cout << "size:";
    std::cin >> size;
    std::cout << "side:(1-buy, 2-sell)";
    std::cin >> side_int;
    if (side_int != 1 and side_int != 2) {
      cout << "wrong side, 1->buy 2->sell!" << endl;
      continue;
    }
    side = (side_int == 1) ? OrderSide::Buy : OrderSide::Sell;
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "Confirm OrderInfo:%s %lf@%d; 1-confirmed other-quit:", ticker.c_str(), price, size);
    std::cout << buffer;
    int confirmed = 0;
    std::cin >> confirmed;
    if (confirmed == 1) {
      Order o;
      snprintf(o.ticker, sizeof(o.ticker), "%s", ticker.c_str());
      o.price = price;
      o.side = side;
      o.size = size;
      snprintf(o.order_ref, sizeof(o.order_ref), "%s%d", ticker.c_str(), count++);
      s->Send(o);
      std::cout << "*****************" <<  o.order_ref << " sent*****************\n";
      sleep(3);
    } else {
      std::cout << "*****************Order Quit!*****************" << std::endl;
      continue;
    }
  }
  exit(1);
}

void ManualController::RunRecv(Recver<ExchangeInfo> * e) {
  ExchangeInfo info;
  while (true) {
    e->Recv(info);
    info.Show(stdout);
  }
}
