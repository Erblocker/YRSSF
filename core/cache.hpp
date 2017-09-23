#ifndef yrssf_core_cache
#define yrssf_core_cache
#include <map>
#include <string>
#include "rwmutex.hpp"
#include "func.hpp"
#include <pthread.h>
#include "global.hpp"
namespace yrssf{
  namespace cache{
    class value{
      unsigned int time;
      unsigned int tl;
      public:
      void(*onfree)(value&);
      void * data;
      unsigned int length;
      std::string v;
      void init(int t){
        time=nowtime_s();
        tl=t;
      }
      inline static int ysabs(int inp){
        if(inp>0)
          return inp;
        else
          return -inp;
      }
      bool timeout(){
        if(ysabs(nowtime_s()-time)>=tl)
          return 1;
      }
      value(){
        init(3600);
        onfree=NULL;
        data=NULL;
        length=0;
      }
      value(unsigned int tt){
        init(tt);
        onfree=NULL;
        data=NULL;
        length=0;
      }
      value(const value & val){
        this->time  =val.time;
        this->tl    =val.tl;
        this->v     =val.v;
        this->onfree=val.onfree;
        this->data  =val.data;
        this->length=val.length;
      }
      value & operator=(const value & val){
        this->time  =val.time;
        this->tl    =val.tl;
        this->v     =val.v;
        this->onfree=val.onfree;
        this->data  =val.data;
        this->length=val.length;
        return *this;
      }
    };
    std::map<std::string,value> cache;
    RWMutex locker;
    int set(lua_State * L){
      if(!lua_isstring (L,1))return 0;
      if(!lua_isstring (L,2))return 0;
      if(!lua_isinteger(L,3))return 0;
      value val(lua_tointeger(L,3));
      val.v=lua_tostring(L,2);
      locker.Wlock();
      cache[lua_tostring(L,1)]=val;
      locker.unlock();
      return 0;
    }
    int read(lua_State * L){
      if(!lua_isstring(L,1))return 0;
      locker.Rlock();
      auto it=cache.find(lua_tostring(L,1));
      if(it==cache.end()){
        locker.unlock();
        return 0;
      }
      value & s=it->second;
      if(s.timeout()){
        locker.unlock();
        return 0;
      }
      lua_pushstring(L,s.v.c_str());
      locker.unlock();
      return 1;
    }
    int del(lua_State * L){
      if(!lua_isstring(L,1))return 0;
      locker.Wlock();
      auto it=cache.find(lua_tostring(L,1));
      if(it!=cache.end())
        cache.erase(it);
      locker.unlock();
      return 0;
    }
    void freetrash(){
      locker.Wlock();
      auto it=cache.begin();
      auto it2=it;
      for(;it!=cache.end();it++){
        bg:
        if(it->second.timeout()){
          it2=it;
          it++;
          auto pp=it->second;
          if(pp.onfree){
            pp.onfree(pp);
          }
          cache.erase(it2);
          goto bg;
        }
      }
      locker.unlock();
    }
  }
}
#endif