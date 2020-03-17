#ifndef SHM_WORKER_HPP_
#define SHM_WORKER_HPP_

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <struct/market_snapshot.h>
#include <atomic>
#include <memory>

using namespace std;

class ShmWorker {
 public:
  ShmWorker() : is_init(false), create_new(false) {

  }

  virtual ~ShmWorker() {
    shmdt(m_data);
    if (create_new) {
      shmctl(shmid, IPC_RMID, 0);
    }
  }


 protected:
  key_t get_keyid(const std::string & name) {
    auto s = "./" + name;
    if (access(s.c_str(), F_OK) == -1) {
      auto s1 = "touch " + s;
      system(s1.c_str());
    }
    key_t semkey = ftok(name.c_str(), 1);
    if (semkey == -1) {
      printf("shm_file:%s not existed\n", s.c_str());
      exit(1);
    }
    return semkey;
  }

  template <typename T>
  void init(std::string name, int size) {
    if (is_init) {
      printf("this shmworker has beed inited!\n");
      return;
    }
    m_key = get_keyid(name);
    shmid = shmget(m_key, 0, 0);
    header_size = 3*sizeof(std::atomic_int) + sizeof(pthread_mutex_t);
    if (shmid == -1) {
      if (errno == ENOENT || errno == EINVAL) {
        int count = 0;
        while (shmid == -1 && count++ < 10) {
          perror("shmget err");
          printf("errno is %s\n", strerror(errno));
          shmid = shmget(m_key, header_size+sizeof(T)*size, 0666 | IPC_CREAT | O_EXCL);
        }
        printf("creating new shm\n");
        create_new = true;
        m_data = (char*)shmat(shmid, NULL, 0);
        m_size = size;
        *reinterpret_cast<atomic_int*>(m_data) = m_size;
        *reinterpret_cast<atomic_int*>(m_data + sizeof(std::atomic_int)) = -1;
        *reinterpret_cast<atomic_int*>(m_data + 2*sizeof(std::atomic_int)) = 0;
        auto p = reinterpret_cast<pthread_mutex_t*>(m_data + 3*sizeof(std::atomic_int));
        pthread_mutexattr_init(&mutexattr);
        pthread_mutexattr_setpshared(&mutexattr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(p, &mutexattr); 
      } else {
        printf("errno is %s\n", strerror(errno));
        exit(1);
      }
    } else {
      m_data = (char*)shmat(shmid, 0, 0);
      m_size = reinterpret_cast<atomic_int*>(m_data)->load();
      printf("connecting to an exsited shm!\n");
    }
  
    if (m_data == (char*)(-1)) {
      printf("shmat failed\n");
      perror("shmat");
      exit(1);
    }
    is_init = true;
  }

  int m_key;
  int m_size;
  int shmid;
  char* m_data;
  bool is_init;
  size_t header_size;
  pthread_mutexattr_t mutexattr;
  bool create_new;
};

#endif  // SHM_WORKER_HPP_
