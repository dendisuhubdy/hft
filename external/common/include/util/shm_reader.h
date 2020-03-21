#include "shm_worker.h"

template <typename T>
class ShmReader : public ShmWorker {
 public:
  ShmReader(const std::string & key, int size);

  ~ShmReader();

  T read();

 private:
  atomic<int> read_index;
  std::unique_ptr<ofstream> f;
};
