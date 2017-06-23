#ifndef yrssf_core_server
#define yrssf_core_server
#include "ysconnection.hpp"
#include "ysdb.hpp"
#include "threadpool.hpp"
#include <stack>
#include <map>
#include "classes.hpp"
namespace yrssf{
class Server;
class requirePool;
class require{
  friend class requirePool;
  public:
    in_addr   from;
    short     port;
    Server *  self;
    char      buffer[sizeof(netSource)];
  protected:
    require * next;
  public:
    require(){
      next=NULL;
    }
    ~require(){
      //ysDebug("free a block");
      if(next) delete next;
    }
};
class requirePool{
  public:
    std::mutex   locker;
    require *    req;
    require *    used;
    int          mallocCount;
    int          usingCount;
    requirePool():locker(){
      req =new require();
      mallocCount=1;
      usingCount=0;
    }
    ~requirePool(){
      if(req) delete req;
      ysDebug("server:mallocCount=%d",mallocCount);
      ysDebug("server:usingCount=%d",usingCount);
      ysDebug("free pool");
    }
    require * get(){
      locker.lock();
      //ysDebug("get chunk");
      auto r=req;
      usingCount++;
      if(req==NULL){
        mallocCount++;
        r=new require();
      }else{
        req=r->next;
      }
      locker.unlock();
      bzero(r,sizeof(require));
      return r;
    }
    void del(require * r){
      locker.lock();
      r->next=req;
      req=r;
      usingCount--;
      //ysDebug("del chunk");
      locker.unlock();
    }
    void freeone(){
      locker.lock();
      auto r=req;
      if(req==NULL){
        locker.unlock();
        return;
      }
      req=r->next;
      mallocCount--;
      r->next=NULL;
      delete r;
      locker.unlock();
    }
}pool;
class Server:public ysConnection{
  bool        isrunning;
  pthread_t   newthread;
  std::mutex  w;
  public:
  Server(short p):ysConnection(p){
    script="server.lua";
    isrunning=1;
  }
  void shutdown(){
    isrunning=0;
    w.lock();
    sleep(3);
    w.unlock();
    ysDebug("server shutdown success\n");
  }
  static void* accept_req(void * req){
    require * r=(require*)req;
    in_addr from=r->from;
    short   port=r->port;
    r->self->run(from,port,r->buffer);
    pool.del(r);
  }
  void runServer(){
    require * r;
    w.lock();
    ysDebug("server running on %d",ntohs(server_addr.sin_port));
    r=pool.get();
    r->self=this;
    while(isrunning){
      if(!wait_for_data(1,0))continue;
      if(recv(&(r->from),&(r->port),r->buffer,sizeof(netSource))){
        //if(pthread_create(&newthread,NULL,accept_req,r)!=0){
        //  perror("pthread_create");
        //}
        threadpool::add(accept_req,r);
        r=pool.get();
        r->self=this;
      }
    }
    pool.del(r);
    w.unlock();
  }
}server(config::L.ysPort);
}
#endif