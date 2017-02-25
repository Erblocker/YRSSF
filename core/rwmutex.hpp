#include <pthread.h>
#ifndef yrssf_rwmutex
#define yrssf_rwmutex
class RWMutex{
  pthread_rwlock_t  m_rw_lock;
  public:
  RWMutex(){
    pthread_rwlock_init(&m_rw_lock,NULL);
  }
  ~RWMutex(){
    pthread_rwlock_destroy(&m_rw_lock);
  }
  void Rlock(){
    pthread_rwlock_rdlock(&m_rw_lock);
  }
  void Runlock(){
    pthread_rwlock_unlock(&m_rw_lock);
  }
  void Wlock(){
    pthread_rwlock_wrlock(&m_rw_lock);
  }
  void Wunlock(){
    pthread_rwlock_unlock(&m_rw_lock);
  }
  void unlock(){
    pthread_rwlock_unlock(&m_rw_lock);
  }
};
#endif