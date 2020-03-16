#ifndef SHM_WRITER_HPP_
#define SHM_WRITER_HPP_

#include "shm_worker.hpp"

template <typename T>
class ShmWriter: public ShmWorker {
 public:
  ShmWriter(const std::string& key, int size) {
    init <T> (key, size);
    mutex = reinterpret_cast<pthread_mutex_t*>(m_data + 3*sizeof(atomic_int));
  }

  ~ShmWriter() {

  }

  void write(const T& shot) {
    pthread_mutex_lock(mutex);
    auto tail = (atomic_int*)(m_data + 2*sizeof(atomic_int));
    memcpy(m_data+header_size+(tail->load()%m_size)*sizeof(T), &shot, sizeof(T));
    tail->fetch_add(1);
    pthread_mutex_unlock(mutex);
  }
 private:
  pthread_mutex_t* mutex;
};

#endif  // SHM_WRITER_HPP_
