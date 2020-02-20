#ifndef DATA_HANDLER_H_
#define DATA_HANDLER_H_

class DataHandler {
 public:
  DataHandler();
  ~DataHandler();
  void LoadData(const std::string& file_path);
  virtual HandleShot(MarketSnapshot* shot) = 0;
}

#endif // DATA_HANDLER_H_
