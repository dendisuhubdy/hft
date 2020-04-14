#include "shm_worker.h"

template <typename T>
class ShmWriter: public ShmWorker {
 public:
  ShmWriter(const std::string & key, int size);

  ~ShmWriter();

  void write(const T& shot);
 private:
  pthread_mutex_t* mutex;
};
