#ifndef yrssf_core_scriptqueue
#define yrssf_core_scriptqueue
#include "global.hpp"
#include <queue>
#include <string>
#include <pthread.h>
#include <mutex>
namespace yrssf{
  class Scriptqueue{
    std::queue<std::string> q;
    std::mutex              locker;
    public:
    bool isrunning;
    Scriptqueue(){}
    ~Scriptqueue(){}
    void insert(std::string & script){
      locker.lock();
      q.push(script);
      locker.unlock();
    }
    void insert(const char * script){
      std::string scr=script;
      insert(scr);
    }
    bool doscript(){
      const char * script;
      if(q.empty())return 0;
      auto Lp=luapool::Create();
      lua_State * L=Lp->L;
      locker.lock();
      std::string str=q.front();
      script=str.c_str();
      q.pop();
      locker.unlock();
      if(luaL_loadbuffer(
          L,
          script,
          str.length(),
          "line"
        )||lua_pcall(L,0,0,0)
      ){
        if(lua_isstring(L,-1))
          ysDebug("%s",lua_tostring(L,-1));
      }
      luapool::Delete(Lp);
      return 1;
    }
    void stop(){
      isrunning=0;
    }
    void autodoscr(){
      isrunning=1;
      while(isrunning){
        if(doscript())continue;
        sleep(1);
      }
    }
  }scriptqueue;
  void * scriptqueuerun_c(void*){
    scriptqueue.autodoscr();
  }
  void scriptqueuerun(){
    pthread_t newthread;
    if(pthread_create(&newthread,NULL,scriptqueuerun_c,NULL)!=0)
      perror("pthread_create");
  }
}
#endif
