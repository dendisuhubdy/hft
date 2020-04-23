#ifndef SRC_ZSFITORDER_TEST_HARNESS_H_
#define SRC_ZSFITORDER_TEST_HARNESS_H_

#include <libconfig.h++>

#include <iostream>
#include <string>

class TestHarness {
 public:
  explicit TestHarness(libconfig::Config* config) {
    std::string address;

    shq::CreateAnonymousTopic(shq::TopicProperties(1000000), &address);
    config->lookup("order_update_path") = address.c_str();
    order_updates_.Connect(address);

    shq::CreateAnonymousTopic(shq::TopicProperties(1000000), &address);
    config->lookup("order_reject_path") = address.c_str();
    order_rejects_.Connect(address);

    shq::CreateAnonymousTopic(shq::TopicProperties(1000000), &address);
    config->lookup("control_path") = address.c_str();
    control_.reset(new shq::Publisher(address));

    shq::CreateAnonymousTopic(shq::TopicProperties(1000000), &address);
    config->lookup("market_orders_path") = address.c_str();
    market_orders_.reset(new shq::Publisher(address));

    shq::CreateAnonymousTopic(shq::TopicProperties(1000000), &address);
    config->lookup("order_handler_update_topic") = address.c_str();
  }

  void Start() {
    is_logging_out_ = false;
    if (pthread_create(&listener_thread_, 0, ListenerThreadEntry, this) != 0) {
      perror("pthread");
      exit(1);
    }
    sleep(1);
    if (pthread_create(&sender_thread_, 0, SenderThreadEntry, this) != 0) {
      perror("pthread");
      exit(1);
    }
  }

 private:
  static void* SenderThreadEntry(void* arg) {
    printf("starting sender thread\n");
    TestHarness* self = static_cast<TestHarness*>(arg);
    self->RunSender();
    return NULL;
  }
  static void* ListenerThreadEntry(void* arg) {
    printf("starting listener thread\n");
    TestHarness* self = static_cast<TestHarness*>(arg);
    self->RunListener();
    return NULL;
  }

  /////////////////////////

  void WaitUntilEnterKey() {
    std::string tmp;
    std::getline(std::cin, tmp);
  }

  void SendNew(int message_id, const char* ticker, const char* cl_order_id, OrderSide::Enum side, int size, double price) {
    printf("Press enter to send NEW order...\n");
    WaitUntilEnterKey();
    RiskManagerOrder order;
    strncpy(order.ticker, ticker, sizeof(order.ticker));
    order.action = OrderAction::NewOrder;
    order.side = side;
    order.size = size;
    order.price = price;
    order.size_prev_filled = 0;
    order.size_prev_left = 0;
    strncpy(order.cl_order_id, cl_order_id, sizeof(order.cl_order_id));
    gettimeofday(&order.time, NULL);
    order.message_id = message_id;

    market_orders_->Send(&order, sizeof(order));
  }

  void SendReplace(int message_id, const char* ticker, const char* cl_order_id, int order_id, OrderSide::Enum side, int size, double price) {
    printf("Press enter to REPLACE order...\n");
    WaitUntilEnterKey();
    RiskManagerOrder order;
    strncpy(order.ticker, ticker, sizeof(order.ticker));
    order.action = OrderAction::ReplaceOrder;
    order.side = side;
    order.size = size;
    order.price = price;
    order.size_prev_filled = 0;
    order.size_prev_left = 0;
    strncpy(order.cl_order_id, cl_order_id, sizeof(order.cl_order_id));
    snprintf(order.order_id, sizeof(order.order_id), "%d", order_id);
    gettimeofday(&order.time, NULL);
    order.message_id = message_id++;

    market_orders_->Send(&order, sizeof(order));
  }
  void SendCancel(int message_id, const char* ticker, int order_id) {
    printf("Press enter to CANCEL order...\n");
    WaitUntilEnterKey();
    RiskManagerOrder order;
    strncpy(order.ticker, ticker, sizeof(order.ticker));
    order.action = OrderAction::CancelOrder;
    snprintf(order.order_id, sizeof(order.order_id), "%d", order_id);
    gettimeofday(&order.time, NULL);
    order.message_id = message_id;

    market_orders_->Send(&order, sizeof(order));
  }

  void RunSender() {
    sleep(1);
    // const char* ticker = "CITICEQ/510330/";

    char action[10];
    int message_id = 1;
    while ((std::cout << "\n action:") && (std::cin >> action)) {
      char ticker[20] = "SFIT/IHM6/";
      std::cout << "\nticker:";
      std::cin >> ticker;
      std::cout << ticker << "\n";

      if (action[0] == 'B' || action[0] == 'S') {
        std::cout << "\nprice:";
        double price;
        std::cin >> price;

        std::cout << "\nsize:";
        int size;
        std::cin >> size;
        if (action[0] == 'B') {
          SendNew(message_id++, ticker, "MYID003", OrderSide::Buy, size, price);  // order id 1
        } else if (action[0] == 'S') {
          SendNew(message_id++, ticker, "MYID003", OrderSide::Sell, size, price);
        }

      } else if (action[0] == 'C') {
        int order_id;
        std::cout << "\norder_id:";
        std::cin >> order_id;
        SendCancel(message_id++, ticker, order_id_);
      }
    }
    // SendNew(message_id++, ticker, "MYID004", OrderSide::Sell, 100, 2.4);  // order id 1
    // SendCancel(message_id++, ticker, order_id_);

    printf("Done! Press enter to Logout...\n");
    WaitUntilEnterKey();
    is_logging_out_ = true;
    control_->Send("LOGOUT", 6);
    sleep(1);
  }

  void RunListener() {
    RiskManagerOrderUpdate update;
    while (!is_logging_out_) {
      if (order_updates_.Receive(&update, shq::kReceiveFlagNoBlock)) {
        printf("------------------------\n");
        printf("reason %s\n", OrderUpdateReason::ToString(update.reason));
        printf("status %s\n", OrderStatus::ToString(update.status));
        printf("side %s\n", update.side == OrderSide::Buy ? "Buy" : "Sell");
        printf("total_size_filled %d\n", update.total_size_filled);
        printf("size_left %d\n", update.size_left);
        printf("price %lf\n", update.price);
        printf("size_filled %d\n", update.size_filled);
        printf("fill_price %lf\n", update.fill_price);
        printf("cl_order_id %s\n", update.cl_order_id);
        printf("order_id %s\n", update.order_id);
        order_id_ = atoi(update.order_id);
      }
      usleep(100000);  // sleep 100ms
    }
    printf("Exiting Listener\n");
  }

 private:
  shq::TopicMessageSubscriber<RiskManagerOrderUpdate> order_updates_;
  shq::TopicMessageSubscriber<RiskManagerOrderUpdate> order_rejects_;
  std::tr1::shared_ptr<shq::Publisher> control_;
  std::tr1::shared_ptr<shq::Publisher> market_orders_;

  volatile bool is_logging_out_;
  pthread_t sender_thread_;
  pthread_t listener_thread_;

  int order_id_;
};

#endif  // SRC_ZSFITORDER_TEST_HARNESS_H_
