#ifndef yrssf_core_serverbase
#define yrssf_core_serverbase
#include "nint.hpp"
#include "func.hpp"
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <pthread.h>
#include <stdarg.h>
namespace yrssf{
class serverBase{
  public:
  struct sockaddr_in   server_addr; 
  int                  server_socket_fd; 
  serverBase(short port){
    bzero(&server_addr, sizeof(server_addr)); 
    server_addr.sin_family = AF_INET; 
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    server_addr.sin_port = htons(port);
    server_socket_fd = socket(AF_INET, SOCK_DGRAM, 0); 
    if(server_socket_fd == -1){
      perror("Create Socket Failed:"); 
      return; 
    }
    if(-1 == (bind(server_socket_fd,(struct sockaddr*)&server_addr,sizeof(server_addr))))  { 
       perror("Server Bind Failed:"); 
       return; 
    }
  }
  ~serverBase(){
    close(server_socket_fd);
  }
  virtual bool recv_b(struct sockaddr_in * client_addr,void *buffer,int size){   //接收
    socklen_t client_addr_length = sizeof(*client_addr); 
    bzero(buffer,size);
    if(recvfrom(server_socket_fd, buffer,size,0,(struct sockaddr*)client_addr,&client_addr_length) == -1) return 0; 
    return 1;
  }
  virtual bool recv(struct sockaddr_in * client_addr,void *buffer,int size){
    return recv_b(client_addr,buffer,size);
  }
  virtual bool send_b(struct sockaddr_in * to,void *buffer,int size){   //发送
    if(sendto(server_socket_fd, buffer, size,0,(struct sockaddr*)to,sizeof(*to)) < 0)return 0;
    return 1; 
  }
  virtual bool send(struct sockaddr_in * to,void *buffer,int size){
    return send_b(to,buffer,size);
  }
  virtual bool recv(in_addr *from,short * port,void *buffer,int size){
     struct sockaddr_in c;
     bool m=recv(&c,buffer,size);
     *from=c.sin_addr;
     *port=ntohs(c.sin_port);
     return m;
  }
  virtual bool send(in_addr to,short port,void *buffer,int size){
     struct sockaddr_in a;
     bzero(&a, sizeof(a));
     a.sin_family = AF_INET; 
     a.sin_addr = to;
     a.sin_port = htons(port);
     return send(&a,buffer,size);
  }
  virtual bool recv_within_time(struct sockaddr_in * addr,void *buf,int buf_n,unsigned int sec,unsigned usec){
     struct timeval tv;
     fd_set readfds;
     int i=0;
     int m;
     socklen_t addr_len = sizeof(*addr);
     unsigned int n=0;
     for(i=0;i<100;i++){
         FD_ZERO(&readfds);
         FD_SET(server_socket_fd,&readfds);
         tv.tv_sec=sec;
         tv.tv_usec=usec;
         m=select(server_socket_fd+1,&readfds,NULL,NULL,&tv);
         if(m==0) continue;
         if(FD_ISSET(server_socket_fd,&readfds))
             if(n=recvfrom(server_socket_fd,buf,buf_n,0,(struct sockaddr*)addr,&addr_len)>=0)
                 return 1;
     }
     return 0;
  }
  bool recv_within_time(in_addr * from,short * port,void * data,int size,int t1,int t2){
    sockaddr_in  s;
    bool m=recv_within_time(&s,data,size,t1,t2);
    *from=s.sin_addr;
    *port=ntohs(s.sin_port);
    return m;
  }
  bool wait_for_data(unsigned int sec,unsigned usec){
     struct timeval tv;
     fd_set readfds;
     int i=0;
     int m;
     unsigned int n=0;
     FD_ZERO(&readfds);
     FD_SET(server_socket_fd,&readfds);
     tv.tv_sec=sec;
     tv.tv_usec=usec;
     m=select(server_socket_fd+1,&readfds,NULL,NULL,&tv);
     if(m==0) return 0;
     if(FD_ISSET(server_socket_fd,&readfds))
         return 1;
     return 0;
  }
};
}
#endif