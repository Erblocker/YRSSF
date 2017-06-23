#ifndef yrssf_threadpool
#define yrssf_threadpool
#include "global.hpp"
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <mutex>
namespace yrssf{
  namespace threadpool{
    typedef struct tpool_work {
       void*               (*routine)(void*);       /* 任务函数 */
       void                *arg;                    /* 传入任务函数的参数 */
       struct tpool_work   *next;                    
    }tpool_work_t;
    class tw_pool{
      tpool_work_t * freed;
      std::mutex locker;
      public:
      tw_pool(){
        freed=NULL;
      }
      ~tw_pool(){
        tpool_work_t * it1;
        tpool_work_t * it=freed;
        while(it){
          it1=it;
          it=it->next;
          delete it1;
        }
      }
      tpool_work_t * get(){
        locker.lock();
        if(freed){
          tpool_work_t * r=freed;
          freed=freed->next;
          locker.unlock();
          return r;
        }else{
          locker.unlock();
          return new tpool_work_t;
        }
      }
      void del(tpool_work_t * f){
        locker.lock();
        f->next=freed;
        freed=f;
        locker.unlock();
      }
    }tpl;
    typedef struct tpool {
       int             shutdown;                    /* 线程池是否销毁 */
       int             max_thr_num;                /* 最大线程数 */
       pthread_t       *thr_id;                    /* 线程ID数组 */
       tpool_work_t    *queue_head;                /* 线程链表 */
       pthread_mutex_t queue_lock;                    
       pthread_cond_t  queue_ready;    
    }tpool_t;
    tpool_t *tpool = NULL;
    void*  thread_routine(void *arg){
      tpool_work_t *work;
      while(1) {
           /* 如果线程池没有被销毁且没有任务要执行，则等待 */
           pthread_mutex_lock(&tpool->queue_lock);
           while(!tpool->queue_head && !tpool->shutdown) {
               pthread_cond_wait(&tpool->queue_ready, &tpool->queue_lock);
           }
           if (tpool->shutdown) {
               pthread_mutex_unlock(&tpool->queue_lock);
               pthread_exit(NULL);
           }
           work = tpool->queue_head;
           tpool->queue_head = tpool->queue_head->next;
           pthread_mutex_unlock(&tpool->queue_lock);
    
           work->routine(work->arg);
           //free(work);
           tpl.del(work);
       }
       return NULL;   
    }
    
    int tpool_create(int max_thr_num){
       int i;
       ysDebug("create thread pool. num=%d",max_thr_num);
       tpool = (tpool_t*)calloc(1, sizeof(tpool_t));
       if (!tpool) {
           printf("%s: calloc failed\n", __FUNCTION__);
           exit(1);
       }
       
       tpool->max_thr_num = max_thr_num;
       tpool->shutdown = 0;
       tpool->queue_head = NULL;
       if (pthread_mutex_init(&tpool->queue_lock, NULL) !=0) {
           printf("%s: pthread_mutex_init failed, errno:%d, error:%s\n",
               __FUNCTION__, errno, strerror(errno));
           exit(1);
       }
       if (pthread_cond_init(&tpool->queue_ready, NULL) !=0 ) {
       printf("%s: pthread_cond_init failed, errno:%d, error:%s\n", 
               __FUNCTION__, errno, strerror(errno));
           exit(1);
       }
       
       /* 创建工作者线程 */
       tpool->thr_id = (pthread_t*)calloc(max_thr_num, sizeof(pthread_t));
       if (!tpool->thr_id) {
           printf("%s: calloc failed\n", __FUNCTION__);
           exit(1);
       }
       for (i = 0; i < max_thr_num; ++i) {
           if (pthread_create(&tpool->thr_id[i], NULL, thread_routine, NULL) != 0){
               printf("%s:pthread_create failed, errno:%d, error:%s\n", __FUNCTION__, 
                   errno, strerror(errno));
               exit(1);
           }
           
       }    
    
       return 0;
    }
    
    void tpool_destroy(){
       int i;
       ysDebug("thread pool shutdown");
       tpool_work_t *member;
   
       if (tpool->shutdown) {
           return;
       }
       tpool->shutdown = 1;
    
       /* 通知所有正在等待的线程 */
       pthread_mutex_lock(&tpool->queue_lock);
       pthread_cond_broadcast(&tpool->queue_ready);
       pthread_mutex_unlock(&tpool->queue_lock);
       for (i = 0; i < tpool->max_thr_num; ++i) {
          pthread_join(tpool->thr_id[i], NULL);
       }
       free(tpool->thr_id);
   
       while(tpool->queue_head) {
          member = tpool->queue_head;
          tpool->queue_head = tpool->queue_head->next;
          free(member);
       }
   
       pthread_mutex_destroy(&tpool->queue_lock);    
       pthread_cond_destroy(&tpool->queue_ready);
   
       free(tpool);    
    }
   
    int tpool_add_work(void*(*routine)(void*), void *arg){
      tpool_work_t *work, *member;
      
      if (!routine){
          printf("%s:Invalid argument\n", __FUNCTION__);
          return -1;
      }
      
      //work = (tpool_work_t*)malloc(sizeof(tpool_work_t));
      work=tpl.get();
      if (!work) {
          printf("%s:malloc failed\n", __FUNCTION__);
          return -1;
      }
      work->routine = routine;
      work->arg = arg;
      work->next = NULL;
   
      pthread_mutex_lock(&tpool->queue_lock);    
      member = tpool->queue_head;
      if (!member) {
          tpool->queue_head = work;
      } else {
          while(member->next) {
              member = member->next;
          }
          member->next = work;
      }
      /* 通知工作者线程，有新任务添加 */
      pthread_cond_signal(&tpool->queue_ready);
      pthread_mutex_unlock(&tpool->queue_lock);
   
          return 0;    
    }
    class Init{
      public:
      Init(){
        if (tpool_create(config::L.tpoolsize) != 0) {
          printf("tpool_create failed\n");
          exit(1);
        }
      }
      ~Init(){
        tpool_destroy();
      }
    }init;
    int add(void*(*routine)(void*), void *arg){
      return tpool_add_work(routine,arg);
    }
  }
}
#endif