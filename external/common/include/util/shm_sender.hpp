#ifndef SHM_SENDER_HPP_
#define SHM_SENDER_HPP_

#include "shm_worker.hpp"

template <typename T>
class ShmSender: public ShmWorker {
 public:
  ShmSender(const std::string& key, int size = 100000) {
    init <T> (key, size);
    mutex = reinterpret_cast<pthread_mutex_t*>(m_data + 3*sizeof(atomic_int));
  }

  ~ShmSender() {

  }

  void Send(const T& shot) {
    pthread_mutex_lock(mutex);
    auto tail = (atomic_int*)(m_data + 2*sizeof(atomic_int));
    memcpy(m_data+header_size+(tail->load()%m_size)*sizeof(T), &shot, sizeof(T));
    tail->fetch_add(1);
    pthread_mutex_unlock(mutex);
  }
 private:
  pthread_mutex_t* mutex;
};

#endif  // SHM_SENDER_HPP_
