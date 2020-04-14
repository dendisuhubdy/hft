#ifndef SHMWORKER_H_
#define SHMWORKER_H_

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include <fcntl.h>
#include <struct/market_snapshot.h>
#include <atomic>
#include <memory>

using namespace std;

class ShmWorker {
 public:
  ShmWorker();

  virtual ~ShmWorker();

 protected:
  template <typename T>
  void init(const std::string & key, int size);

  int m_key;
  int m_size;
  int shmid;
  char* m_data;
  bool is_init;
  size_t header_size;
  pthread_mutexattr_t mutexattr;
};
#endif  // SHMWORKER_H_
