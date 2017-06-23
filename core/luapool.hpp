#ifndef yrssf_luapool
#define yrssf_luapool
#include <mutex>
#include <list>
extern "C" {
#include "lua/src/lua.h"
#include "lua/src/lualib.h"
#include "lua/src/lauxlib.h"
#include "lua/src/luaconf.h"
}
namespace yrssf{
  namespace luapool{
    std::list< void(*)(lua_State*) > reg;
    void initstate(lua_State * L){
      auto it=reg.begin();
      for(;it!=reg.end();it++){
        (*it)(L);
      }
    }
    class pool;
    class lua_node{
      friend class pool;
      protected:
      lua_node *next;
      public:
      lua_State * L;
      lua_node(){
        L=luaL_newstate();
        
        initstate(L);
        
        next=NULL;
        
      }
      void clean(){
        lua_settop(L,0);
      }
      ~lua_node(){
        lua_close(L);
        //if(next)
        //  delete next;
      }
    };
    class pool{
      lua_node * freed;
      std::mutex locker;
      unsigned int used,malloced;
      public:
      pool(){
        freed=NULL;
        used=0;
        malloced=0;
      }
      ~pool(){
        lua_node * it1;
        lua_node * it=freed;
        while(it){
          it1=it;
          it=it->next;
          delete it1;
        }
        printf("debug: %s:%d %s() luapool:mallocCount=%d \n",__FILE__, __LINE__, __FUNCTION__,malloced);
        printf("debug: %s:%d %s() luapool:usingCount=%d \n",__FILE__, __LINE__, __FUNCTION__,used);
      }
      lua_node * get(){
        locker.lock();
        used++;
        if(freed){
          lua_node * r=freed;
          freed=freed->next;
          locker.unlock();
          r->next=NULL;
          
          return r;
        }else{
          malloced++;
          locker.unlock();
          return new lua_node;
        }
      }
      void del(lua_node * f){
        locker.lock();
        f->next=freed;
        freed=f;
        used--;
        locker.unlock();
      }
    }p;
    typedef lua_node luap;
    luap * Create(){
      return p.get();
    }
    void Delete(luap * t){
      t->clean();
      p.del(t);
    }
  }
}
#endif