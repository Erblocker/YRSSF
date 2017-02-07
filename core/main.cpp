#define SERVER_PORT         1215
#define CLIENT_PORT         8001
#define SOURCE_CHUNK_SIZE   4096
#define _SUCCEED         0x30
#define _FAIL            0x31
#define _NEWSRC          0x32
#define _GETSRC          0x33
#define _DELSRC          0x34
#define _SETSRC_APPEND   0x35
#define _SETACL          0x36
#define _SETPWD          0x37
#define _LOGIN           0x38
#define _NEWUSER         0x39
#define _PLUS            0x3A
#define _GETSRCNAME      0x3B
#define _DELUSER         0x3C
#define _CONNECTUSER     0x60
#define _P2PCONNECT      0x61
#define _P2PCONNECT_C    0x62
#define _LOGIP           0x63
#define _GOLAST          0x64
#define _SHELL           0xBB
#define _GETSERVERINFO   0xAA
#define _CONNECT         0xAB
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netinet/in.h> 
#include <unistd.h>
#include <errno.h>
#include <netdb.h> 
#include <stdarg.h>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <memory>
#include "leveldb/db.h"
#include "leveldb/write_batch.h"
extern "C" {
#include "lua/src/lua.h"
#include "lua/src/lualib.h"
#include "lua/src/lauxlib.h"
#include "lua/src/luaconf.h"
#include "lwan.h"
#include "lwan-serve-files.h"
}
#include <cassert>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <string.h>
#include <map>
#include <set>
#include <stack>
#include <math.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <sys/prctl.h>
#include <sqlite3.h>
namespace yrssf{
//////////////////////////////////
void   int2str(int32_t  * i,char * c){
  sprintf(c, "%8x ", *i);
  c[8]='\0';
}
void   str2int(char * c,int32_t  * i){
  sscanf(c,  "%8x",  i); 
}
int randnum(){
  static int st=0;
  int r;
  st++;
  srand(st);
  r=rand();
  if(!r)r++;
  return r;
}
char * randstr(){
  static char result[17];
  result[16]='\0';
  char arr[]="xcvbnm,.asdfghjkl;'qwertyuiop1234567890~`/[]{}";
  for(int i=0;i<16;i++){
    result[i]=arr[randnum() % sizeof(arr)];
  }
}
char * nowtime(){
  static char timestr[9];
  timestr[8]='\0';
  time_t t;  
  t = time(NULL);  
  struct tm *lt;  
  int ii = time(&t);
  int2str(&ii,timestr);
  return timestr;
}
int nowtime_s(){
  time_t t;  
  t = time(NULL);  
  struct tm *lt;  
  return time(&t);
}
int iptoint( const char *ip ){
  return ntohl( inet_addr( ip ) );
}
/*void inttoip( int ip_num, char *ip ){
  strcpy( ip, (char*)inet_ntoa( htonl( ip_num ) ) );
}*/
void wristr(const char * in,char * out){
    for(int i=0;i<16;i++){
      if(in[i]=='\0'){
        out[i]='\0';
        return;
      }
      out[i]=in[i];
    }
}
class location{
  public:
  in_addr ip;
  short   port;
  location(){
    ip.s_addr = htonl(INADDR_ANY); 
    port = 0; 
  }
  location(const location & i){
    ip=i.ip;
    port=i.port;
  }
  location & operator=(const location & i){
    ip=i.ip;
    port=i.port;
    return *this;
  }
};
struct netHeader{
  char     crypto;
  int32_t  userid;
  int32_t  unique;
  char     password[16];
  char     mode;
  char     globalMode;
  char     function[16];
};
struct netQuery{
  netHeader header;
  char      str1[16];
  char      str2[16];
  in_addr   addr;
  int32_t   num1;
  int32_t   num2;
  int32_t   num3;
  int32_t   num4;
};
struct netSource{
  netHeader header;
  u_int32_t size;
  char      title[16];
  char      source[SOURCE_CHUNK_SIZE];
};
const char defaultUserInfo[]="1234567890abcdef1234567890abcdef1234567890abcdef";
lua_State * gblua;
class YsDB{
  public:
  leveldb::DB *          user;
  leveldb::DB *          userTime;
  leveldb::DB *          userTmp;
  std::map<int32_t,location> userIP;
  leveldb::DB *          sourceBoardcast;
  leveldb::DB *          sourceUser;
  leveldb::DB *          keys;
  class userunique{
    public:
    std::mutex        locker;
    std::set<int32_t> uni;
    userunique(){}
    userunique(const userunique & i){}
  };
  
  std::map<int32_t,userunique> unique;
  std::mutex                   uniquelocker;
  std::mutex                   locker;
/*
* format:
**************************************************************
* name             *   key         *    value                *
**************************************************************
* user:            * {username[8]} *  {password[16]}{acl[16]}*
* userTime:        * {username[8]} *    {time}               *
* userIP           * {username[8]} *    {ip}                 *
* userTmp          * {username[8]} *    {pwd}                *
* sourceBoardcast: * {srcname[8]}  *    {string}             *
* sourceUser:      * {srcname[8]}  *    {string}             *
**************************************************************
*
*Acl:
*position                       enum
*[0]        edit source         t,f
*[1]        edit all user       t,f
*[2]        node mode           t,f
*[3]        shell               t,f
*/
  YsDB(){
    leveldb::Options options;
    options.create_if_missing = true;
    assert(leveldb::DB::Open(options, "data/user", &user).ok());
    assert(leveldb::DB::Open(options, "data/userTime", &userTime).ok());
    assert(leveldb::DB::Open(options, "data/userTmp", &userTmp).ok());
    assert(leveldb::DB::Open(options, "data/sourceUser", &sourceUser).ok());
    assert(leveldb::DB::Open(options, "data/sourceBoardcast", &sourceBoardcast).ok());
    assert(leveldb::DB::Open(options, "data/keys", &keys).ok());
  }
  ~YsDB(){
    delete user;
    delete userTime;
    delete userTmp;
    delete sourceBoardcast;
    delete sourceUser;
    delete keys;
  }
  void getkey(){}
  void setkey(){}
  bool logunique(int32_t uid,int32_t lid){
    if(lid==0)return 1;
    uniquelocker.lock();
    userunique * u=&unique[uid];
    uniquelocker.unlock();
    bool result;
    u->locker.lock();
    if(u->uni.find(lid)==u->uni.end()){
      u->uni.insert(lid);
      result=1;
    }else{
      result=0;
    }
    u->locker.unlock();
    return result;
  }
  bool login(int32_t userid,const char * pwd,std::string * v){
    char name[9];
    const char * tp;
    int2str(&userid,name);
    if(!userTmp->Get(leveldb::ReadOptions(),name,v).ok()) goto loginTmpFail;
    if(v->empty())goto loginTmpFail;
    tp=v->c_str();
    for(int i=0;i<16;i++){
      if(pwd[i]==tp[i] && pwd[i]=='\0' && i>6)
        break;
      if(pwd[i]=='\0') goto loginTmpFail;
      if( tp[i]=='\0') goto loginTmpFail;
      if(pwd[i]!=tp[i])goto loginTmpFail;
    }
    if(!user->Get(leveldb::ReadOptions(),name,v).ok()) return 0;
    goto loginsucceed;
    loginTmpFail:
    if(!user->Get(leveldb::ReadOptions(),name,v).ok()) return 0;
    if(v->empty())return 0;
    tp=v->c_str();
    for(int i=0;i<16;i++){
      if(pwd[i]==tp[i] && pwd[i]=='\0' && i>6)
        break;
      if(pwd[i]=='\0') return 0;
      if( tp[i]=='\0') return 0;
      if(pwd[i]!=tp[i])return 0;
    }
    loginsucceed:
    userTime->Put(leveldb::WriteOptions(),name,nowtime());
    return 1;
  }
  bool setTmpPwd(int32_t userid,char * pwd){
    int i;
    for(i=0;i<16;i++)if(pwd[i]=='\0')return 0;
    char v[17];
    char name[9];
    int2str(&userid,name);
    for(i=0;i<16;i++){
      v[i]=pwd[i];
    }
    v[16]='\0';
    userTmp->Put(leveldb::WriteOptions(),name,v);
    return 1;
  }
  template<typename T1,typename T2>
  void writestr(T1 a,T2 b,int l,int begin){
    int i;
    for(i=0;i<l;i++){
      a[i+begin]=b[i];
    }
  }
  template<typename T>
  bool strlencheck(T in,int l){
    int i;
    for(i=0;i<l;i++)if(in[i]=='\0')return 0;
    return 1;
  }
  bool changePwd(int32_t userid,const char * pwd){
    int i;
    for(i=0;i<16;i++)if(pwd[i]=='\0')return 0;
    std::string v;
    char name[9];
    int2str(&userid,name);
    if(!user->Get(leveldb::ReadOptions(),name,&v).ok())return 0;
    if(v.length()<sizeof(defaultUserInfo)) return 0;
    for(i=0;i<16;i++)v[i]=pwd[i];
    user->Put(leveldb::WriteOptions(),name,v);
    return 1;
  }
  bool changeAcl(int32_t userid,const char * acl){
    int i;
    for(i=0;i<16;i++)if(acl[i]=='\0')return 0;
    std::string v;
    char name[9];
    int2str(&userid,name);
    if(!user->Get(leveldb::ReadOptions(),name,&v).ok())return 0;
    if(v.length()<sizeof(defaultUserInfo)) return 0;
    for(i=0;i<16;i++)v[i+16]=acl[i];
    user->Put(leveldb::WriteOptions(),name,v);
    return 1;
  }
  bool newUser(int32_t userid){
    char name[9];
    int2str(&userid,name);
    user->Put(leveldb::WriteOptions(),name,defaultUserInfo);
  }
  bool delUser(int32_t userid){
    char name[9];
    int2str(&userid,name);
    user->Delete(leveldb::WriteOptions(),name);
  }
  bool getSrcName(char * out,const char * name){
    char str[9];
    int i;
    for(i=0;i<16;i++)if(name[i]=='\0')return 0;
    for(i=0;i<8;i++){
      out[i]=name[i];
    }
    out[8]='\0';
    return 1;
  }
  bool getSrcName(char * out,const char * name,int32_t userid){
    char str[17];
    int2str(&userid,str);
    int i;
    for(i=0;i<16;i++)if(name[i]=='\0')return 0;
    for(i=0;i<8;i++){
      out[i+8]=name[i];
    }
    out[16]='\0';
    return 1;
  }
  bool writeSrc(const char * sname,const char * src){
    char name[9];
    if(!getSrcName(name,sname))return 0;
    return sourceBoardcast->Put(leveldb::WriteOptions(),name,src).ok();
  }
  bool writeSrc(const char * sname,int32_t userid,const char * src){
    char name1[9];
    std::string v;
    if(!getSrcName(name1,sname))return 0;
    if(!sourceBoardcast->Get(leveldb::ReadOptions(),name1,&v).ok())return 0;
    char name[17];
    if(!getSrcName(name,sname,userid))return 0;
    return sourceUser->Put(leveldb::WriteOptions(),name,src).ok();
  }
  bool readSrc(const char * sname,std::string * src){
    char name[9];
    if(!getSrcName(name,sname))return 0;
    return sourceBoardcast->Get(leveldb::ReadOptions(),name,src).ok();
  }
  bool readSrc(const char * sname,int32_t userid,std::string * src){
    char name1[9];
    std::string v;
    if(!getSrcName(name1,sname))return 0;
    if(!sourceBoardcast->Get(leveldb::ReadOptions(),name1,&v).ok())return 0;
    char name[17];
    if(!getSrcName(name,sname,userid))return 0;
    return sourceUser->Get(leveldb::ReadOptions(),name,src).ok();
  }
  bool delSrc(const char * sname){
    char name[9];
    if(!getSrcName(name,sname))return 0;
    return sourceBoardcast->Delete(leveldb::WriteOptions(),name).ok();
  }
  bool delSrc(const char * sname,int32_t userid){
    char name1[9];
    std::string v;
    if(!getSrcName(name1,sname))return 0;
    if(!sourceBoardcast->Get(leveldb::ReadOptions(),name1,&v).ok())return 0;
    char name[17];
    if(!getSrcName(name,sname,userid))return 0;
    return sourceUser->Delete(leveldb::WriteOptions(),name).ok();
  }
  bool existSrc(const char * sname){
    std::string str;
    if(!readSrc(sname,&str)) return 0;
    if(str.empty()) return 0;
    return 1;
  }
  bool existSrc(const char * sname,int32_t userid){
    std::string str;
    if(!readSrc(sname,userid,&str)) return 0;
    if(str.empty()) return 0;
    return 1;
  }
  void logIP(int32_t userid,in_addr ip,short port){
    locker.lock();
    location p;
    p.ip=ip;
    p.port=port;
    userIP[userid]=p;
    locker.unlock();
  }
  bool getUserIP(int32_t userid,in_addr *ip,short *port){
    locker.lock();
    location &p=userIP[userid];
    *ip=p.ip;
    *port=p.port;
    locker.unlock();
    if(p.port==0)
      return 0;
    else
      return 1;
  }
}ysDB;
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
  virtual bool recv(struct sockaddr_in * client_addr,void *buffer,int size){   //接收
    socklen_t client_addr_length = sizeof(*client_addr); 
    bzero(buffer,size);
    if(recvfrom(server_socket_fd, buffer,size,0,(struct sockaddr*)client_addr,&client_addr_length) == -1) return 0; 
    return 1;
  }
  virtual bool send(struct sockaddr_in * to,void *buffer,int size){   //发送
    if(sendto(server_socket_fd, buffer, size,0,(struct sockaddr*)to,sizeof(*to)) < 0)return 0;
    return 1; 
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
class ysConnection:public serverBase{
  public:
  const char   *       script;
  int32_t              myuserid;
  char                 mypassword[17];
  in_addr              parIP;
  short                parPort;
  std::stack<location> ipstack;
  std::mutex           iplocker;
  ysConnection(short p):serverBase(p){
    mypassword[16]='\0';
    script="default.lua";
  }
  void fileappend(char * n,char * data,int size){
    FILE * f=fopen(n,"a");
    fwrite(data,size,1,f);
    fclose(f);
  }
  void createfile(char *n){
    FILE * f=fopen(n,"w");
    fclose(f);
  }
  virtual bool checkInParent(int32_t,const char*){
    return 0;
  }
  virtual bool succeed(in_addr to,short port){
    netQuery data;
    bzero(&data, sizeof(data));
    data.header.userid=myuserid;
    for(int i=0;i<16;i++)
      data.header.password[i]=mypassword[i];
    data.header.mode=_SUCCEED;
    return send(to,port,&data,sizeof(data));
  }
  virtual bool fail(in_addr to,short port){
    netQuery data;
    bzero(&data, sizeof(data));
    data.header.userid=myuserid;
    for(int i=0;i<16;i++)
      data.header.password[i]=mypassword[i];
    data.header.mode=_FAIL;
    return send(to,port,&data,sizeof(data));
  }
  virtual void connectUser(int32_t uid,in_addr from,short port){
     netQuery qypk;
     in_addr  ab;
     short    pb;
     bzero(&qypk,sizeof(qypk));
     ysDB.getUserIP(uid,&ab,&pb);
     for(int i=0;i<8;i++){
          qypk.header.mode=_P2PCONNECT;
          qypk.addr=ab;
          qypk.num1=pb;
          send(from,port,&qypk,sizeof(qypk));
          qypk.header.mode=_P2PCONNECT_C;
          qypk.addr=from;
          qypk.num1=port;
          send(ab  ,pb  ,&qypk,sizeof(qypk));
     }
  }
  virtual bool p2pconnect(in_addr to,short port){
    int              i;
    int              m;
    int              t           =nowtime_s();
    netQuery         buf;
    fd_set           readfds;
    struct timeval   tv;
    sockaddr_in      s;
    socklen_t        addr_len    =sizeof(s);
    netQuery         data;
    bzero(&data, sizeof(data));
    data.header.mode=_CONNECT;
    while(abs(nowtime_s()-t)<10){
         send(to,port,&data,sizeof(data));
         FD_ZERO(&readfds);
         FD_SET(server_socket_fd,&readfds);
         tv.tv_sec=1;
         tv.tv_usec=0;
         m=select(server_socket_fd+1,&readfds,NULL,NULL,&tv);
         if(m==0) continue;
         if(FD_ISSET(server_socket_fd,&readfds)){
           if(recvfrom(server_socket_fd,&buf,sizeof(buf),0,(struct sockaddr*)&s,&addr_len)>=0){
             if(s.sin_addr.s_addr==to.s_addr){
               if(ntohs(s.sin_port)==port){
                 return 1;
               }
             }
           }
         }
    }
    return 0;
  }
  virtual bool changeParentServer(in_addr addr,short port){
    iplocker.lock();
    if(p2pconnect(addr,port)){
      location lc;
      lc.ip  =parIP;
      lc.port=parPort;
      parIP  =addr;
      parPort=port;
      ipstack.push(lc);
      iplocker.unlock();
      return 1;
    }else
      return 0;
  }
  virtual bool goLast(){
    location lc;
    iplocker.lock();
    if(ipstack.empty())
      return 0;
    lc=ipstack.top();
    ipstack.pop();
    parIP=lc.ip;
    parPort=lc.port;
    iplocker.unlock();
    return 1;
  }
  static void funcreg(lua_State * lua){
    lua_register(lua,"succeed",[](lua_State * L){
      if(!lua_isstring(L,1))return 0;
      in_addr ip;
      ip.s_addr=inet_addr(lua_tostring(L,1));
      if(!lua_isinteger(L,2))return 0;
      short port=lua_tointeger(L,2);
      if(!lua_isinteger(L,3))return 0;
      auto self=(ysConnection*)lua_tointeger(L,3);
      self->succeed(ip,port);
      return 0;
    });
    lua_register(lua,"fail",[](lua_State * L){
      if(!lua_isstring(L,1))return 0;
      in_addr ip;
      ip.s_addr=inet_addr(lua_tostring(L,1));
      if(!lua_isinteger(L,2))return 0;
      short port=lua_tointeger(L,2);
      if(!lua_isinteger(L,3))return 0;
      auto self=(ysConnection*)lua_tointeger(L,3);
      self->fail(ip,port);
      return 0;
    });
    lua_register(lua,"connect",[](lua_State * L){
      if(!lua_isstring(L,1))return 0;
      in_addr ip;
      ip.s_addr=inet_addr(lua_tostring(L,1));
      if(!lua_isinteger(L,2))return 0;
      short port=lua_tointeger(L,2);
      if(!lua_isinteger(L,3))return 0;
      auto self=(ysConnection*)lua_tointeger(L,3);
      lua_pushboolean(L,self->p2pconnect(ip,port));
      return 1;
    });
    lua_register(lua,"send",[](lua_State * L){
      if(!lua_isstring(L,1))return 0;
      in_addr ip;
      ip.s_addr=inet_addr(lua_tostring(L,1));
      if(!lua_isinteger(L,2))return 0;
      short port=lua_tointeger(L,2);
      if(!lua_isinteger(L,3))return 0;
      auto self=(ysConnection*)lua_tointeger(L,3);
      netQuery query;
      bzero(&query,sizeof(query));
      query.header.mode=_PLUS;
      query.header.userid=self->myuserid;
      wristr(self->mypassword,query.header.password);
      char buf[17];
      if(lua_isstring(L,4))
        wristr(lua_tostring(L,4),query.str1);
      if(lua_isstring(L,5))
        wristr(lua_tostring(L,5),query.str2);
      if(lua_isinteger(L,6))
        query.num1=lua_tointeger(L,6);
      if(lua_isinteger(L,7))
        query.num2=lua_tointeger(L,7);
      if(lua_isinteger(L,8))
        query.num3=lua_tointeger(L,8);
      if(lua_isinteger(L,9))
        query.num4=lua_tointeger(L,9);
      lua_pushboolean(L,(self->send(ip,port,&query,sizeof(netQuery))));
      return 1;
    });
    lua_register(lua,"recv",[](lua_State * L){
      in_addr  from;
      short    port;
      netQuery buf;
      char     bufs[17];
      bufs[16]='\0';
      if(!lua_isinteger(L,1))return 0;
      bzero(&buf,sizeof(buf));
      auto self=(ysConnection*)lua_tointeger(L,1);
      if(self->recv_within_time(&from,&port,&buf,sizeof(buf),3,0)){
        lua_pushstring (L,inet_ntoa(from));
        lua_pushinteger(L,port);
        //push userid
        lua_pushinteger(L,buf.header.userid);
        //push password
        wristr(buf.header.password,bufs);
        lua_pushstring(L,bufs);
        //push querys
        wristr(buf.str1, bufs);
        lua_pushstring(L,bufs);
        wristr(buf.str2, bufs);
        lua_pushstring(L,bufs);
        lua_pushinteger(L,buf.num1);
        lua_pushinteger(L,buf.num2);
        lua_pushinteger(L,buf.num3);
        lua_pushinteger(L,buf.num4);
        return 10;
      }else{
        lua_pushboolean(L,0);
        return 1;
      }
    });
    lua_register(lua,"trylogin",[](lua_State * L){
      if(!lua_isinteger(L,1))return 0;
      if(!lua_isstring (L,2))return 0;
      int uid=lua_tointeger(L,1);
      const char * pwd=lua_tostring(L,2);
      std::string v;
      lua_pushboolean(L,(ysDB.login(uid,pwd,&v)));
      return 1;
    });
    lua_register(lua,"changePwd",[](lua_State * L){
      if(!lua_isinteger(L,1))return 0;
      if(!lua_isstring (L,2))return 0;
      const char * text=lua_tostring(L,2);
      lua_pushboolean(L,ysDB.changePwd(lua_tointeger(L,1),text));
      return 1;
    });
    lua_register(lua,"changeAcl",[](lua_State * L){
      if(!lua_isinteger(L,1))return 0;
      if(!lua_isstring (L,2))return 0;
      const char * text=lua_tostring(L,2);
      lua_pushboolean(L,ysDB.changeAcl(lua_tointeger(L,1),text));
      return 1;
    });
    lua_register(lua,"readSrc",[](lua_State * L){
      if(!lua_isstring (L,1))return 0;
      std::string result;
      if(lua_isinteger (L,2)){
        ysDB.readSrc(lua_tostring(L,1),lua_tointeger(L,2),&result);
      }else{
        ysDB.readSrc(lua_tostring(L,1),&result);
      }
      lua_pushstring(L,result.c_str());
      return 1;
    });
    lua_register(lua,"writeSrc",[](lua_State * L){
      if(!lua_isstring (L,1))return 0;
      if(!lua_isstring (L,2))return 0;
      if(lua_isinteger(L,3)){
        lua_pushboolean(L,ysDB.writeSrc(lua_tostring(L,1),lua_tointeger(L,3),lua_tostring(L,2)));
      }else{
        lua_pushboolean(L,ysDB.writeSrc(lua_tostring(L,1),lua_tostring(L,2)));
      }
      return 1;
    });
    lua_register(lua,"delSrc",[](lua_State * L){
      bool m;
      if(!lua_isstring (L,1))return 0;
      if(lua_isinteger(L,2)){
        m=ysDB.delSrc(lua_tostring(L,1),lua_tointeger(L,2));
      }else{
        m=ysDB.delSrc(lua_tostring(L,1));
      }
      lua_pushboolean(L,m);
      return 1;
    });
  }
  virtual bool plus(in_addr from,short fport,void * data,std::string * u){
    lua_State * lua;
    int port=(int)fport;
    auto query=(netQuery*)data;
    char buffer[17];
    buffer[16]='\0';
    lua=lua_newthread(gblua);
    lua_pushstring (lua,inet_ntoa(from));
    lua_setglobal(lua,"FROM");
    lua_pushinteger(lua,port);
    lua_setglobal(lua,"FROM_PORT");
    lua_pushinteger(lua,(int)this);
    lua_setglobal(lua,"RUNNINGSERVER");
    lua_pushinteger(lua,query->header.userid);
    lua_setglobal(lua,"USERID");
    wristr(query->header.function,buffer);
    lua_pushstring(lua,buffer);
    lua_setglobal(lua,"FUNCTION_NAME");
    lua_pushstring(lua,u->c_str());
    lua_setglobal(lua,"USERINFO");
    funcreg(lua);
    luaL_dofile(lua,script);
    lua_close(lua);
  }
  virtual bool run(in_addr from,short port,void * data){
    std::string userinfo;
    int i;
    static int num;
    num++;
    in_addr ab;
    short   pb;
    netHeader * header = (netHeader*)data;
    netSource * source = (netSource*)data;
    netQuery  * query  =  (netQuery*)data;
    if(from.s_addr==parIP.s_addr)
    if(port==parPort){
      if(header->mode==_LOGIN){
        ysDB.newUser(header->userid);
        ysDB.changePwd(header->userid,header->password);
        ysDB.changeAcl(header->userid,"ttftffffffffffff");
        return 1;
      }else
      if(header->mode==_P2PCONNECT){
        iplocker.lock();
        if(from.s_addr==parIP.s_addr)
        if(port==parPort)
        if(p2pconnect(query->addr,query->num1)){
          location lc;
          lc.ip  =parIP;
          lc.port=parPort;
          parIP  =query->addr;
          parPort=query->num1;
          ipstack.push(lc);
          iplocker.unlock();
          return 1;
        }else{
          iplocker.unlock();
          return 0;
        }
      }else
      if(header->mode==_P2PCONNECT_C){
        p2pconnect(query->addr,query->num1);
      }else
      if(header->mode==_GOLAST){
        goLast();
      }
    }
    if(header->mode==_GETSERVERINFO){
      netQuery q;
      bzero(&q,sizeof(q));
      q.header.mode=_LOGIN;
      q.header.userid=myuserid;
      for(i=0;i<16;i++)
        q.header.password[i]=mypassword[i];
      send(from,port,&q,sizeof(q));
      return 1;
    }
    if(!ysDB.login(header->userid,header->password,&userinfo)){
      if(!checkInParent(header->userid,header->password)){
        fail(from,port);
        return 0;
      }
    }
    if(!ysDB.logunique(header->userid,header->unique)){
      fail(from,port);
      return 0;
    }
    char filename[]="static/srcs/ttttttttuuuuuuuurrrrrrrr.yss";
    const char * filenamer;
    netSource respk;
    netQuery  qypk;
    char * time;
    char   name[9];
    char   number[9];
    const char * tp;
    std::string result;
    int2str(&(header->userid),name);
    int2str(&num,number);
    FILE * ff;
    //begin : 12
    //end   : 35
    switch(header->mode){
      case _NEWSRC:
        //get file name
          time==nowtime();
          for(i=0;i<8;i++){
            filename[i+12]=time[i];
          }
          for(i=0;i<8;i++){
            filename[i+20]=name[i];
          }
          for(i=0;i<8;i++){
            filename[i+28]=number[i];
          }
        //get file name end
        //write database
        if(header->globalMode=='t'){
          if(userinfo.at(16)=='t'){
            if(!ysDB.existSrc(query->str1))
            if(ysDB.writeSrc(query->str1,filename)){
              succeed(from,port);
              createfile(filename);
              return 1;
            }
          }
        }else{
          if(!ysDB.existSrc(query->str1,header->userid))
          if(ysDB.writeSrc(query->str1,header->userid,filename)){
            succeed(from,port);
            createfile(filename);
            return 1;
          }
        }
        fail(from,port);
        return 0;
      break;
      case _SETSRC_APPEND:
        //get file name
        if((source->size)>SOURCE_CHUNK_SIZE || (source->size)<0){
          fail(from,port);
          return 0;
        }
        if(header->globalMode=='t'){
          if(userinfo.at(16)=='t'){
            if(!ysDB.readSrc(source->title,&result)){
              fail(from,port);
              return 0;
            }
          }
        }else{
          if(!ysDB.readSrc(source->title,header->userid,&result)){
            fail(from,port);
            return 0;
          }
        }
        filenamer=result.c_str();
        fileappend(filename,source->source,source->size);
        //get file name end
        succeed(from,port);
        return 1;
      break;
      case _GETSRC:
        //get file name
        if(header->globalMode=='t'){
            if(!ysDB.readSrc(query->str1,&result)){
              fail(from,port);
              return 0;
            }
        }else{
          if(!ysDB.readSrc(query->str1,header->userid,&result)){
            fail(from,port);
            return 0;
          }
        }
        filenamer=result.c_str();
        //get file name end
        bzero(&respk,sizeof(respk));
        ff=fopen(filenamer,"r");
        fseek(ff,SOURCE_CHUNK_SIZE*query->num1,SEEK_SET);
        for(i=0;(i<SOURCE_CHUNK_SIZE && !feof(ff));i++){
          respk.source[i]=fgetc(ff);
        }
        fclose(ff);
        respk.header.userid=myuserid;
        for(i=0;i<16;i++)
          respk.header.password[i]=mypassword[i];
        respk.header.unique=randnum();
        respk.size=i;
        respk.header.mode=_SETSRC_APPEND;
        wristr(query->str1,respk.title);
        send(from,port,&respk,sizeof(respk));
        return 1;
      break;
      case _GETSRCNAME:
        //get file name
        if(header->globalMode=='t'){
            if(!ysDB.readSrc(query->str1,&result)){
              fail(from,port);
              return 0;
            }
        }else{
          if(!ysDB.readSrc(query->str1,header->userid,&result)){
            fail(from,port);
            return 0;
          }
        }
        filenamer=result.c_str();
        //get file name end
        bzero(&respk,sizeof(respk));
        for(i=0;i<SOURCE_CHUNK_SIZE;i++){
          respk.source[i]=filenamer[i];
          if(filenamer[i]=='\0')
            break;
        }
        respk.size=i;
        respk.header.userid=myuserid;
        for(i=0;i<16;i++)
          respk.header.password[i]=mypassword[i];
        send(from,port,&respk,sizeof(respk));
        return 1;
      break;
      case _DELSRC:
        if(header->globalMode=='t'){
          if(userinfo.at(16)=='t'){
            if(ysDB.readSrc(query->str1,&result)){
                      ysDB.delSrc(query->str1);
            }else{
              fail(from,port);
              return 0;
            }
          }
        }else{
          if(ysDB.readSrc(query->str1,header->userid,&result)){
                    ysDB.delSrc(query->str1,header->userid);
          }else{
            fail(from,port);
            return 0;
          }
        }
        if(remove(result.c_str())==-1){
          fail(from,port);
          return 0;
        }
        succeed(from,port);
        return 1;
      break;
      case _NEWUSER:
        if(userinfo.at(17)!='t'){
          fail(from,port);
          return 0;
        }
        ysDB.newUser(query->num1);
        succeed(from,port);
        return 1;
      break;
      case _DELUSER:
        if(userinfo.at(17)!='t'){
          fail(from,port);
          return 0;
        }
        ysDB.delUser(query->num1);
        succeed(from,port);
        return 1;
      break;
      case _SETACL:
        if(userinfo.at(17)!='t'){
          fail(from,port);
          return 0;
        }
        if(ysDB.changeAcl(query->num1,query->str1)){
          succeed(from,port);
          return 1;
        }else{
          fail(from,port);
          return 0;
        }
      break;
      case _SETPWD:
        if(header->globalMode=='t'){
          if(userinfo.at(17)!='t'){
            fail(from,port);
            return 0;
          }
          if(ysDB.changePwd(query->num1,query->str1)){
            succeed(from,port);
            return 1;
          }else{
            fail(from,port);
            return 0;
          }
        }else{
          if(!ysDB.user->Get(leveldb::ReadOptions(),name,&result).ok()){fail(from,port);return 0;}
          if(result.empty()){fail(from,port);return 0;}
          tp=result.c_str();
          for(int i=0;i<16;i++){
            if(query->str2[i]=='\0') {fail(from,port);return 0;}
            if(         tp[i]=='\0') {fail(from,port);return 0;}
            if(query->str2[i]!=tp[i]){fail(from,port);return 0;}
          }
          if(ysDB.changePwd(header->userid,query->str1)){
            succeed(from,port);
            return 1;
          }else{
            fail(from,port);
            return 0;
          }
        }
      break;
      case _LOGIN:
        if(ysDB.setTmpPwd(header->userid,query->str1)){
          succeed(from,port);
          return 1;
        }else{
          fail(from,port);
          return 0;
        }
      break;
      case _LOGIP:
        if(userinfo.at(18)!='t'){
          fail(from,port);
          return 0;
        }  
        ysDB.logIP(header->userid,from,port);
        succeed(from,port);
        return 1;
      break;
      case _CONNECTUSER:
        connectUser(query->num1,from,port);
        return 1;
      break;
      case _SHELL:
        if(userinfo.at(19)!='t'){
          fail(from,port);
          return 0;
        }
        system(source->source);
        succeed(from,port);
        return 1;
      break;
      case _PLUS:
      return plus(from,port,data,&userinfo);
      break;
      case _SUCCEED: break;
      case _FAIL   : break;
    }
    return 0;
  }
  virtual bool login(){
/*
*       yrssf log:
*check password
*set temperor password
*change password to temperor password
*get server info
*do something......
*/
    netQuery qypk;
    netQuery buf;
    int      i;
    in_addr  from;
    short    port;
    bzero(&qypk,sizeof(qypk));
    bzero(&buf ,sizeof(buf ));
    qypk.header.mode=_LOGIN;
    wristr(mypassword,qypk.header.password);
    qypk.header.userid=myuserid;
    wristr(randstr(),mypassword);
    wristr(mypassword,qypk.str1);
    //qypk.header.globalMode=globalmode;
    for(i=0;i<10;i++){
      send(parIP,parPort,&qypk,sizeof(qypk));
      loginloop1:
      if(recv_within_time(&from,&port,&buf,sizeof(buf),1,0)){
        if(from.s_addr==parIP.s_addr && port==parPort){
          if(buf.header.mode==_SUCCEED)
            goto loginloop1end;
          else
            return 0;
        }
        else
          goto loginloop1;
      }
    }
    return 0;
    loginloop1end:
    bzero(qypk.str1,16);
    wristr(mypassword,qypk.header.password);
    for(i=0;i<10;i++){
      send(parIP,parPort,&qypk,sizeof(qypk));
      loginloop2:
      if(recv_within_time(&from,&port,&buf,sizeof(buf),1,0)){
        if(from.s_addr==parIP.s_addr && port==parPort){
          if(buf.header.mode==_LOGIN)
            goto loginloop2end;
          else
            return 0;
        }
        else
          goto loginloop2;
      }
    }
    return 0;
    loginloop2end:
    return run(parIP,parPort,&buf);
  }
};
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
      if(next) delete next;
    }
};
class requirePool{
  public:
    std::mutex   locker;
    require *    req;
    require *    used;
    requirePool():locker(){
      req =NULL;
      used=NULL;
    }
    ~requirePool(){
      if(req) delete req;
      if(used)delete used;
    }
    require * get(){
      locker.lock();
      auto r=req;
      if(req==NULL){
        r=new require();
      }else{
        req=r->next;
      }
      locker.unlock();
      bzero(r,sizeof(require));
      r->next=used;
      used=r;
      return r;
    }
    void del(require * r){
      locker.lock();
      r->next=req;
      req=r;
      locker.unlock();
    }
}pool;
class Server:public ysConnection{
  bool        isrunning;
  pthread_t   newthread;
  std::mutex  wmx;
  public:
  Server(short p):ysConnection(p){
    script="server.lua";
    isrunning=1;
  }
  void shutdown(){
    isrunning=0;
    wmx.lock();
    wmx.unlock();
    printf("\033[40;43mYRSSF:\033[0mserver shutdown success\n");
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
    wmx.lock();
    std::cout << "\033[40;43mYRSSF:\033[0mserver running on \0"<<ntohs(server_addr.sin_port)<< std::endl;
    while(isrunning){
      r=pool.get();
      r->self=this;
      if(!wait_for_data(1,0))continue;
      if(recv(&(r->from),&(r->port),r->buffer,sizeof(netSource))){
        if(pthread_create(&newthread,NULL,accept_req,r)!=0){
          perror("pthread_create");
          pool.del(r);
        }
      }
    }
    wmx.unlock();
  }
}server(SERVER_PORT);
class Client:public Server{
  public:
  char  globalmode;
  Client(short p):Server(p){
    globalmode='f';
    script="client.lua";
  }
  bool newSrc(const char * sname){
    netQuery qypk;
    netQuery buf;
    int      i;
    in_addr  from;
    short    port;
    bzero(&qypk,sizeof(qypk));
    bzero(&buf ,sizeof(buf ));
    qypk.header.mode=_NEWSRC;
    wristr(mypassword,qypk.header.password);
    for(i=0;i<8;i++)
      qypk.str1[i]=sname[i];
    qypk.header.userid=myuserid;
    qypk.header.globalMode=globalmode;
    for(i=0;i<10;i++){
      send(parIP,parPort,&qypk,sizeof(qypk));
      dsloop1:
      if(recv_within_time(&from,&port,&buf,sizeof(buf),1,0)){
        if(from.s_addr==parIP.s_addr && port==parPort){
          if(buf.header.mode==_SUCCEED)
            return 1;
          else
            return 0;
        }
        else
          goto dsloop1;
      }
    }
  }
  bool downloadchunk(const char * sname,FILE * path,int32_t s){
    netQuery  qypk;
    netSource buf;
    int      i;
    in_addr  from;
    short    port;
    bzero(&qypk,sizeof(qypk));
    bzero(&buf ,sizeof(buf ));
    qypk.header.mode=_GETSRC;
    qypk.header.userid=myuserid;
    qypk.num1=s;
    for(i=0;i<8;i++)
      qypk.str1[i]=sname[i];
    qypk.header.globalMode=globalmode;
    wristr(mypassword,qypk.header.password);
    for(i=0;i<10;i++){
      send(parIP,parPort,&qypk,sizeof(qypk));
      dcloop1:
      if(recv_within_time(&from,&port,&buf,sizeof(buf),1,0)){
        if(from.s_addr==parIP.s_addr && port==parPort){
          if(buf.header.mode==_SETSRC_APPEND){
            int size;
            size=buf.size>SOURCE_CHUNK_SIZE ? SOURCE_CHUNK_SIZE : buf.size;
            size=size>0 ? size : 0;
            fwrite(buf.source,size,1,path);
            if(buf.size==SOURCE_CHUNK_SIZE)
              return 1;
            else
              return 0;
          }else
            return 0;
        }
        else
          goto dcloop1;
      }
    }
    return 0;
  }
  void download(const char * sname,const char * path){
    FILE * f=fopen(path,"a");
    int i=0;
    while(downloadchunk(sname,f,i))i++;
    fclose(f);
  }
  bool uploadchunk(const char *sname,const char *buf2,int32_t size){
    netSource qypk;
    netQuery  buf;
    int      i;
    in_addr  from;
    short    port;
    bzero(&qypk,sizeof(qypk));
    bzero(&buf ,sizeof(buf ));
    qypk.header.mode=_SETSRC_APPEND;
    qypk.header.userid=myuserid;
    int r=1;
    qypk.header.unique=randnum();
    qypk.size=size;
    for(i=0;i<8;i++)
      qypk.title[i]=sname[i];
    qypk.header.globalMode=globalmode;
    wristr(mypassword,qypk.header.password);
    for(i=0;(i<size && i<SOURCE_CHUNK_SIZE);i++){
      qypk.source[i]=buf2[i];
    }
    for(i=0;i<10;i++){
      send(parIP,parPort,&qypk,sizeof(qypk));
      ucloop2:
      if(recv_within_time(&from,&port,&buf,sizeof(buf),1,0)){
        if(from.s_addr==parIP.s_addr && port==parPort){
          if(buf.header.mode==_SUCCEED){
            return 1;
          }else
            return 0;
        }
        else
          goto ucloop2;
      }
    }
    return 0;
  }
  void upload(const char * sname,const char * path){
    int i;
    char buf[SOURCE_CHUNK_SIZE];
    FILE * ff=fopen(path,"r");
    while(!feof(ff)){
      for(i=0;(i<SOURCE_CHUNK_SIZE && !feof(ff));i++)
        buf[i]=fgetc(ff);
      if(!uploadchunk(sname,buf,i))return;
    }
    fclose(ff);
  }
  bool del(char * sname){
    netQuery qypk;
    netQuery buf;
    int      i;
    in_addr  from;
    short    port;
    bzero(&qypk,sizeof(qypk));
    bzero(&buf ,sizeof(buf ));
    qypk.header.mode=_DELSRC;
    for(i=0;i<8;i++)
      qypk.str1[i]=sname[i];
    wristr(mypassword,qypk.header.password);
    qypk.header.userid=myuserid;
    qypk.header.globalMode=globalmode;
    for(i=0;i<10;i++){
      send(parIP,parPort,&qypk,sizeof(qypk));
      dsloop1:
      if(recv_within_time(&from,&port,&buf,sizeof(buf),1,0)){
        if(from.s_addr==parIP.s_addr && port==parPort){
          if(buf.header.mode==_SUCCEED)
            return 1;
          else
            return 0;
        }
        else
          goto dsloop1;
      }
    }
  }
  bool setPwd(const char * pwd,const char * lpwd){
    netQuery qypk;
    netQuery buf;
    int      i;
    in_addr  from;
    short    port;
    bzero(&qypk,sizeof(qypk));
    bzero(&buf ,sizeof(buf ));
    qypk.header.mode=_SETPWD;
    wristr(mypassword,qypk.header.password);
    wristr(pwd       ,qypk.str1);
    wristr(lpwd      ,qypk.str2);
    qypk.header.userid=myuserid;
    for(i=0;i<10;i++){
      send(parIP,parPort,&qypk,sizeof(qypk));
      dsloop1:
      if(recv_within_time(&from,&port,&buf,sizeof(buf),1,0)){
        if(from.s_addr==parIP.s_addr && port==parPort){
          if(buf.header.mode==_SUCCEED)
            return 1;
          else
            return 0;
        }
        else
          goto dsloop1;
      }
    }
  }
  bool setPwd(const char * pwd,int32_t puserid){
    netQuery qypk;
    netQuery buf;
    int      i;
    in_addr  from;
    short    port;
    bzero(&qypk,sizeof(qypk));
    bzero(&buf ,sizeof(buf ));
    qypk.header.mode=_SETPWD;
    wristr(mypassword,qypk.header.password);
    wristr(pwd       ,qypk.str1);
    qypk.header.userid=myuserid;
    qypk.header.globalMode='t';
    qypk.num1=puserid;
    for(i=0;i<10;i++){
      send(parIP,parPort,&qypk,sizeof(qypk));
      dsloop1:
      if(recv_within_time(&from,&port,&buf,sizeof(buf),1,0)){
        if(from.s_addr==parIP.s_addr && port==parPort){
          if(buf.header.mode==_SUCCEED)
            return 1;
          else
            return 0;
        }
        else
          goto dsloop1;
      }
    }
  }
  bool newUser(int32_t uid){
    netQuery qypk;
    netQuery buf;
    int      i;
    in_addr  from;
    short    port;
    bzero(&qypk,sizeof(qypk));
    bzero(&buf ,sizeof(buf ));
    qypk.header.mode=_NEWUSER;
    qypk.num1=uid;
    wristr(mypassword,qypk.header.password);
    qypk.header.userid=myuserid;
    qypk.header.globalMode='t';
    for(i=0;i<10;i++){
      send(parIP,parPort,&qypk,sizeof(qypk));
      dsloop1:
      if(recv_within_time(&from,&port,&buf,sizeof(buf),1,0)){
        if(from.s_addr==parIP.s_addr && port==parPort){
          if(buf.header.mode==_SUCCEED)
            return 1;
          else
            return 0;
        }
        else
          goto dsloop1;
      }
    }
  }
}client(CLIENT_PORT);
sqlite3 * db;
class API{
  public:
  API(){
    db=NULL;
    printf("\033[40;43mYRSSF:\033[0mdatabase loading...\n");
    int ret = sqlite3_open("./data/yrssf.db", &db);
    if( ret != SQLITE_OK ) {
      printf("can not open %s \n", sqlite3_errmsg(db));
      return;
    }
  }
  ~API(){
    if(db)sqlite3_close(db);
  }
  struct runsqlcbs{
    lua_State * lua;
    int i;
  };
  static int runsql_callback(
    void * d,
    int col_count,
    char ** col_values,
    char ** col_Name
  ){
    runsqlcbs * data=(runsqlcbs*)d;
    lua_State * L=data->lua;
    lua_createtable(L,col_count,0);//create new array
    lua_rawseti(L,-2,data->i);
    lua_pushnumber(L,-1);
    lua_rawseti(L,-2,0);  //fill array[i][0]
    for(int i=0;i<col_count;i++){
      lua_pushstring(L,col_values[i]);
      lua_rawseti(L,-2,i+1);
    }
    data->i++;
    return 0;
  }
  static int runsql(lua_State * L){
    char * pErrMsg;
    int ret;
    if(!lua_isstring(L,1))return 0;
    const char * sSQL1=lua_tostring(L,1);
    lua_newtable(L);//create main array
    lua_pushnumber(L,-1);
    lua_rawseti(L,-2,0);  //fill array[0]
    runsqlcbs d;
    d.lua=L;
    d.i=1;
    ret=sqlite3_exec( db, sSQL1, runsql_callback, &d, &pErrMsg );
    if( ret != SQLITE_OK ){
      lua_pop(L,1);
      lua_pushstring(L,pErrMsg);
      lua_pushboolean(L,0);
      sqlite3_free(pErrMsg);
      return 2;
    }else{
      lua_pushboolean(L,1);
      return 2;
    }
  }
  static int lua_upload(lua_State * L){
      char str[9];
      str[8]='\0';
      if(!lua_isstring(L,1))return 0;
      if(!lua_isstring(L,2))return 0;
      const char * sname=lua_tostring(L,1);
      for(int i=0;i<8;i++){
        str[i]=sname[i];
        if(sname[i]=='\0')
          break;
      }
      client.upload(str,lua_tostring(L,2));
      return 0;
  }
  static int lua_download(lua_State * L){
      char str[9];
      str[8]='\0';
      if(!lua_isstring(L,1))return 0;
      if(!lua_isstring(L,2))return 0;
      const char * sname=lua_tostring(L,1);
      for(int i=0;i<8;i++){
        str[i]=sname[i];
        if(sname[i]=='\0')
          break;
      }
      client.download(str,lua_tostring(L,2));
      return 0;
  }
  static int lua_del(lua_State * L){
      char str[9];
      str[8]='\0';
      if(!lua_isstring(L,1))return 0;
      const char * sname=lua_tostring(L,1);
      for(int i=0;i<8;i++){
        str[i]=sname[i];
        if(sname[i]=='\0')
          break;
      }
      client.del(str);
      return 0;
  }
  static int lua_setPwd(lua_State * L){
      if(lua_isstring(L,1)){
        if(!lua_isstring(L,2))return 0;
        client.setPwd(lua_tostring(L,1),lua_tostring(L,2));
        return 0;
      }else
      if(lua_isinteger(L,1)){
        if(!lua_isstring(L,2))return 0;
        client.setPwd(lua_tostring(L,2),(int32_t)lua_tointeger(L,1));
        return 0;
      }
  }
  static int lua_newUser(lua_State * L){
      if(!lua_isinteger(L,1))return 0;
      client.newUser(lua_tointeger(L,1));
      return 0;
  }
  static int lua_cps(lua_State * L){
      if(!lua_isstring (L,1))return 0;
      if(!lua_isinteger(L,2))return 0;
      in_addr addr;
      addr.s_addr=inet_addr(lua_tostring(L,1));
      lua_pushboolean(L,client.changeParentServer(addr,lua_tointeger(L,2)));
      return 1;
  }
  static int lua_ssu(lua_State * L){
      if(!lua_isstring (L,2))return 0;
      if(!lua_isinteger(L,1))return 0;
      server.myuserid=lua_tointeger(L,1);
      wristr(lua_tostring(L,2),server.mypassword);
      return 0;
  }
  static int lua_scu(lua_State * L){
      if(!lua_isstring (L,2))return 0;
      if(!lua_isinteger(L,1))return 0;
      client.myuserid=lua_tointeger(L,1);
      wristr(lua_tostring(L,2),client.mypassword);
      return 0;
  }
  static int lua_connectUser(lua_State * L){
    in_addr addr;
    if(!lua_isinteger(L,1))return 0;
    if(!lua_isstring (L,2))return 0;
    if(!lua_isinteger(L,3))return 0;
    addr.s_addr=inet_addr(lua_tostring(L,2));
    server.connectUser(lua_tointeger(L,1),addr,lua_tointeger(L,3));
    return 0;
  }
  void funcreg(lua_State * L){
    lua_register(L,"clientUpload",        lua_upload);
    lua_register(L,"clientDownload",      lua_download);
    lua_register(L,"clientDel",           lua_del);
    lua_register(L,"clientSetPwd",        lua_setPwd);
    lua_register(L,"clientNewUser",       lua_newUser);
    lua_register(L,"changeParentServer",  lua_cps);
    lua_register(L,"connectUser",         lua_connectUser);
    lua_register(L,"runsql",              runsql);
    lua_register(L,"globalModeOn",[](lua_State * L){
      client.globalmode='t';
      return 0;
    });
    lua_register(L,"globalModeOff",[](lua_State * L){
      client.globalmode='f';
      return 0;
    });
    lua_register(L,"setServerUser",lua_ssu);
    lua_register(L,"setClientUser",lua_scu);
  }
}api;
class Init{
  public:
  static void* runServerThread(void*){
    server.runServer();
  }
  void run(){
    pthread_t newthread;
    printf("\033[40;43mYRSSF:\033[0m");
    printf("\033[40;36mYRSSF create thread\033[0m\n");
    if(pthread_create(&newthread,NULL,runServerThread,NULL)!=0)
      perror("pthread_create");
  }
  Init(){
    printf("\033[40;43mYRSSF:\033[0m\033[40;36mYRSSF init\033[0m\n");
    gblua=luaL_newstate();
    luaL_openlibs(gblua);
    lua_pushinteger(gblua,(int)&server);
    lua_setglobal(gblua,"SERVER");
    lua_pushinteger(gblua,(int)&client);
    lua_setglobal(gblua,"CLIENT");
    ysConnection::funcreg(gblua);
    api.funcreg(gblua);
  }
  ~Init(){
    printf("\033[40;43mYRSSF:\033[0m\033[40;36mYRSSF server shutdown\033[0m\n");
    server.shutdown();
    lua_close(gblua);
  }
}init;
///////////////////////////
}//////////////////////////
///////////////////////////
extern "C" void runServer(){
    yrssf::init.run();
}
extern "C" void shutdownServer(){
    yrssf::init.~Init();
}
extern "C" void loadAPI(lua_State * L){
    lua_pushinteger(L,(int)&yrssf::server);
    lua_setglobal(L,"SERVER");
    lua_pushinteger(L,(int)&yrssf::client);
    lua_setglobal(L,"CLIENT");
    yrssf::ysConnection::funcreg(L);
    yrssf::api.funcreg(L);
}
extern "C" int luaopen_yrssf(lua_State * L){
    loadAPI(L);
    return 1;
}
extern "C" void dofile(const char * path){
    lua_State * L=lua_newthread(yrssf::gblua);
    luaL_dofile(L,path);
}
static lwan_http_status_t ajax(lwan_request_t *request,lwan_response_t *response, void *data){
    int i,l;
    lua_State * L=lua_newthread(yrssf::gblua);
    lwan_key_value_t *headers = (lwan_key_value_t*)coro_malloc(request->conn->coro, sizeof(*headers) * 2);
    if (UNLIKELY(!headers))
        return HTTP_INTERNAL_ERROR;
    headers[0].key   = (char*)"Cache-Control";
    headers[0].value = (char*)"no-cache";
    headers[1].key   = NULL;
    headers[1].value = NULL;
    const char * message;
    const static char Emessage[]="NULL";
    response->mime_type = "text/html;charset=utf-8";
    response->headers   = headers;
    lua_createtable(L,0,request->query_params.len);
    for(i=0;i<request->query_params.len;i++){
      lua_pushstring(L, request->query_params.base[i].key);
      lua_pushstring(L, request->query_params.base[i].value);
      lua_settable(L, -3);
    }
    lua_setglobal(L,"GET");
    lua_createtable(L,0,request->post_data.len);
    for(i=0;i<request->post_data.len;i++){
      lua_pushstring(L, request->post_data.base[i].key);
      lua_pushstring(L, request->post_data.base[i].value);
      lua_settable(L, -3);
    }
    lua_setglobal(L,"POST");
    lua_createtable(L,0,request->cookies.len);
    for(i=0;i<request->cookies.len;i++){
      lua_pushstring(L, request->cookies.base[i].key);
      lua_pushstring(L, request->cookies.base[i].value);
      lua_settable(L, -3);
    }
    lua_setglobal(L,"COOKIE");
    luaL_dofile(L,"ajax.lua");
    lua_getglobal(L,"RESULT");
    if(lua_isstring(L,-1)){
      message=lua_tostring(L,-1);
      l=strlen(message);
      strbuf_set(response->buffer, message,l);
      return HTTP_OK;
    }else{
      l=sizeof(Emessage);
      strbuf_set_static(response->buffer,Emessage,l-1);
      return HTTP_OK;
    }
}
void init_daemon(){
  int pid;
  int i; 
  if(pid=fork()) 
    exit(0);
  else
    if(pid< 0) 
      exit(1);
  setsid();
  if(pid=fork()) 
    exit(0);
  else
    if(pid< 0) 
      exit(1);
  for(i=0;i< NOFILE;++i)
    close(i); 
  //chdir("/tmp");
  umask(0);
  return; 
}
extern "C" int main(){
    lwan_url_map_t default_map[3];
    default_map[0].prefix = "/ajax";
    default_map[0].handler = ajax;
    default_map[1].prefix = "/";
    default_map[1].module = lwan_module_serve_files();
    default_map[1].flags = (lwan_handler_flags_t)0;
    struct lwan_serve_files_settings_t servefile;
    default_map[1].args = &servefile;
    servefile.root_path                        = "static";
    servefile.index_html                       = "index.html";
    servefile.serve_precompressed_files        = true;
    servefile.directory_list_template          = NULL;
    servefile.auto_index                       = true;
    default_map[2].prefix = NULL;
    lwan_t l;
    lwan_init(&l);
    lwan_set_url_map(&l, default_map);
    runServer();
    lwan_main_loop(&l);
    printf("\033[40;43mYRSSF:\033[0m\033[40;37mYRSSF shutdown\033[0m\n");
    
    default_map[0].prefix  = NULL;
    default_map[0].handler = NULL;
    default_map[1].prefix  = NULL;
    default_map[1].module  = NULL;
    default_map[1].flags   = (lwan_handler_flags_t)0;
    default_map[1].args    = NULL;
    
    shutdownServer();
    printf("\033[40;43mYRSSF:\033[0m\033[40;37mweb server shutdown\033[0m\n");
    lwan_shutdown(&l);
    printf("\033[40;43mYRSSF:\033[0m\033[40;37mweb shutdown success\033[0m\n");
    return 0;
}