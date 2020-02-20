#include "order.h"
#include <vector>
#include <string>
#include <algorithm>

bool cmp(Order a, Order b) {
  return a.price < b.price;
}

class Matcher {
 public:
  Matcher(){
   ask_book.clear();
   bid_book.clear();
  }
  std::string Insert(Order& o) {
    std::make_heap(bid_book.begin(), bid_book.end(), cmp);
    return "";
  }

 private:
  std::vector<Order>ask_book;
  std::vector<Order>bid_book;
};
