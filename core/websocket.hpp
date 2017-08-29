#ifndef yrssf_core_websocket
#define yrssf_core_websocket
#include "httpd.hpp"
#include "global.hpp"
#include <list>
namespace yrssf{
  namespace websocket{
    class Websocket{
      public:
      std::list<int> fds;
      Websocket(){
      }
      ~Websocket(){
        for(auto fd:fds){
          close(fd);
        }
      }
      void add(int fd){
        fds.push_back(fd);
      }
    }wsock;
    int boardcast(void * data,int len){
      for(auto itb=wsock.fds.begin();itb!=wsock.fds.end();){
        auto it=itb++;
        int fd=*it;
        if(send(fd, data, len, 0)==-1){
          close(fd);
          wsock.fds.erase(it);
        }
      }
    }
    bool(*onWebsocketConnect)(httpd::request *)=[](httpd::request *){
      return false;
    };
    void callback(httpd::request * req){
      if(onWebsocketConnect==NULL)
        close(req->fd);
      if(!onWebsocketConnect(req))
        close(req->fd);
      wsock.add(req->fd);
    }
  }
}
#endif