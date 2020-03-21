#ifndef SHM_RECVER_HPP_
#define SHM_RECVER_HPP_

#include "shm_worker.hpp"

template <typename T>
class ShmRecver : public ShmWorker {
 public:
  ShmRecver(const std::string & key, int size = 100000) {
    init <T> (key, size);
    sleep(1);
    auto tail = (atomic_int*)(m_data + 2*sizeof(atomic_int));
    read_index = tail->load();
    printf("read_index is %d\n", read_index.load());
  }

  ~ShmRecver() {

  }

  inline void Recv(T& t) {
    while (read_index.load() == ((atomic_int*)(m_data + 2*sizeof(int)))->load()) {
    }
    t = *reinterpret_cast<T*>(m_data+header_size+(read_index.load()%m_size)*sizeof(T));
    read_index.fetch_add(1);
  }

 private:
  atomic<int> read_index;
  std::unique_ptr<ofstream> f;
};

#endif // SHM_RECVER_HPP_
