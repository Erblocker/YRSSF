#ifndef yrssf_core_daemon
#define yrssf_core_daemon
#include "key.hpp"
#include "client.hpp"
#include <mutex>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <arpa/inet.h>
namespace yrssf{
struct{
  char mode;
  int32_t num1,num2,num3,num4;
  char str1[16];
  char str2[16];
}loopsendconf;
class Loopsend{
  public:
  static void * send(void *){
    netQuery loopSendData;
    while(1){
      bzero(&loopSendData,sizeof(loopSendData));
      
      loopSendData.header.mode=loopsendconf.mode;
      
      loopSendData.num1=loopsendconf.num1;
      loopSendData.num2=loopsendconf.num2;
      loopSendData.num3=loopsendconf.num3;
      loopSendData.num4=loopsendconf.num4;
      
      wristr(loopsendconf.str1,loopSendData.str1);
      wristr(loopsendconf.str2,loopSendData.str2);
      
      wristr(client.mypassword,loopSendData.header.password);
      loopSendData.header.userid=client.myuserid;
      loopSendData.header.unique=randnum();
      if(client.iscrypt)crypt_encode(&loopSendData,&client.key);
      client.send(client.parIP , client.parPort , &loopSendData ,sizeof(loopSendData));
      sleep(4);
    }
  }
  Loopsend(){
    loopsendconf.mode=_CONNECT;
    pthread_t newthread;
    if(pthread_create(&newthread,NULL,send,NULL)!=0)
      perror("pthread_create");
  }
}loopsend;
}
#endif