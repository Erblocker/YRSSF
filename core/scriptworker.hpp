#ifndef yrssf_core_scriptworker
#define yrssf_core_scriptworker
#include "global.hpp"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <mutex>
#include <atomic>
namespace yrssf{
  namespace sworker{
    class worker{
      public:
      pthread_t thread;
      std::string  str;
      worker(){
      }
    };
    void * create_s(void * s){
      auto self=(worker*)s;
      auto L=lua_newthread(gblua);
      std::string & script=self->str;
      if(luaL_loadbuffer(
          L,
          script.c_str(),
          script.length(),
          "line"
        )||lua_pcall(L,0,0,0)
      ){
        if(lua_isstring(L,-1))
          ysDebug("%s",lua_tostring(L,-1));
      }
      delete self;
    }
    int create(lua_State * L){
      if(!lua_isstring(L,1))return 0;
      auto * m=new worker;
      m->str=lua_tostring(L,1);
      if(pthread_create(&m->thread,NULL,create_s,m)!=0){
        perror("pthread_create");
        return 0;
      }
      return 0;
    }
  }
}
#endif