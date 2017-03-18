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
#define _UPDATEKEY       0xAC
#define _LIVESRC         0xAD
#define _LIVE            0xAE
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <arpa/inet.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/stat.h>
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
#include "aes/aes.h"
#include "zlib.h"
}
#include "ecc.hpp"
#include "rwmutex.hpp"
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
#include <list>
#include <math.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <sys/prctl.h>
#include <dlfcn.h>
#include <sqlite3.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
namespace yrssf{
//////////////////////////////////
const char defaultUserInfo[]="1234567890abcdef1234567890abcdef1234567890abcdef";
lua_State * gblua;
class Libs{
  std::list<void*> liblist;
  public:
  Libs(){}
  ~Libs(){
    std::list<void*>::iterator i;
    for(i=liblist.begin(); i!=liblist.end();i++)
      dlclose(*i);
  }
  void open(const char * path){
    void * handler=dlopen(path,RTLD_LAZY);
    if(handler==NULL){
      printf("fail to open %s \n" ,path);
      return;
    }
    
    liblist.push_back(handler);
    
    auto func=(void(*)(lua_State *))dlsym(handler,"lua_open");
    const char * err=dlerror();
    
    if(err)printf("Error:%s\n",err);
    
    if(func)func(gblua);
    
  }
}libs;
void   int2str(int32_t  * i,char * c){
  sprintf(c, "%8x ", *i);
  c[8]='\0';
}
void   str2int(char * c,int32_t  * i){
  sscanf(c,  "%8x",  i); 
}
int randnum(){
  static int st=time(NULL);
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
class nint32{
  public:
  int32_t value;
  nint32(){
    value=0;
  }
  nint32(int32_t v){
    value=htonl(v);
  }
  nint32 & operator=(int32_t v){
    value=htonl(v);
    return *this;
  }
  nint32(const nint32 & v){
    value=v.value;
  }
  nint32 & operator=(const nint32 & v){
    value=v.value;
    return *this;
  }
  int32_t val(){
    return ntohl(value);
  }
  int32_t operator()(){
    return val();
  }
};
class nint64{
  private:
  uint32_t a,b;
  public:
  uint64_t get(){
    return ntohl(a) * 0x100000000 + ntohl(b);
  }
  void set(uint64_t value){
    uint32_t s,z;
    s=value     / 0x100000000;
    z=value - s * 0x100000000;
    a=htonl(s);
    b=htonl(z);
  }
  nint64(){
    set(0);
  }
  nint64(const nint64 & v){
    a=v.a;
    b=v.b;
  }
  nint64 & operator=(const nint64 & v){
    a=v.a;
    b=v.b;
    return *this;
  }
  nint64(int64_t v){
    set(v);
  }
  nint64 & operator=(int64_t v){
    set(v);
    return *this;
  }
  int64_t operator()(){
    return get();
  }
};
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
  char     crypt;
  nint32   userid;
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
  nint32    num1;
  nint32    num2;
  nint32    num3;
  nint32    num4;
  char      endchunk[16];
};
struct netSource{
  netHeader header;
  nint32    size;
  char      title[16];
  char      source[SOURCE_CHUNK_SIZE];
  char      endchunk[16];
};
class Key:public ECC{
  public:
  struct write{};
  struct read{};
  struct netSendkey{
    nint64 p,a,b;
    nint64 kx,ky,px,py;
    char key[32];
  }*buf;
  char key[16];
  void setkey(){
    buf->p=e.p;
    buf->a=e.a;
    buf->b=e.b;
    buf->px=pare.x;
    buf->py=pare.y;
    buf->kx=publickey.x;
    buf->ky=publickey.y;
    char * rk=randstr();
    int i;
    for(i=0;i<16;i++){
      key[i]=rk[i];
    }
    int64_t data[2];
    char * cp=(char*)data;
    for(i=0;i<16;i++){
      cp[i]=key[i];
    }
    Pare po(pare);
    po.x=data[0];
    po.y=data[1];
    ECC::Message cryptout = encryption(po);
    cp=(char*)&cryptout;
    for(i=0;i<sizeof(cryptout);i++){
      this->buf->key[i]=cp[i];
    }
  }
  void getkey(){
    int64_t * data=(int64_t*)this->buf->key;
    ECC::Pare pp(pare);
    ECC::Message cryptin(pp,pp);
    cryptin.c1.x=data[0];
    cryptin.c1.y=data[1];
    cryptin.c2.x=data[2];
    cryptin.c2.y=data[3];
    ECC::Pare po(pare);
    po=decryption(cryptin);
    int64_t * ot=(int64_t*)key;
    ot[0]=po.x;
    ot[1]=po.y;
  }
  Key(){}
  Key(netSendkey * sk,read):ECC(ECC::E(sk->p(),sk->a(),sk->b()),sk->kx(),sk->ky(),sk->px(),sk->py()){
    //read mode
    buf=sk;
  }
  Key(netSendkey * sk,int64_t pvk):ECC(ECC::E(sk->p(),sk->a(),sk->b()),sk->kx(),sk->ky(),sk->px(),sk->py()){
    //read mode
    buf=sk;
    privatekey=pvk;
    getkey();
  }
  Key(netSendkey * sk):ECC(){
    //send mode
    buf=sk;
    buf->p=e.p;
    buf->a=e.a;
    buf->b=e.b;
    buf->px=pare.x;
    buf->py=pare.y;
    buf->kx=publickey.x;
    buf->ky=publickey.y;
  }
};
struct aesblock{
  uint8_t data[16];
  aesblock & operator=(aesblock & f){
    for(int i=0;i<16;i++) data[i]=f.data[i];
    return *this;
  }
};
template<typename T>
void crypt_encode(T data,aesblock * key){
  if(data->header.crypt=='t')return;
  register aesblock * here =(aesblock*)&(data->header.unique);
  register aesblock * end  =(aesblock*)&(data->endchunk[0]);
  aesblock buf;
  data->header.crypt='t';
  while(here<end){
    AES128_ECB_encrypt(here->data, key->data, buf.data);
    memcpy(here,&buf,sizeof(aesblock));
    here++;
  }
}
template<typename T>
void crypt_decode(T data,aesblock * key){
  if(data->header.crypt!='t')return;
  register aesblock * here =(aesblock*)&(data->header.unique);
  register aesblock * end  =(aesblock*)&(data->endchunk[0]);
  aesblock buf;
  data->header.crypt='f';
  while(here<end){
    AES128_ECB_decrypt(here->data, key->data, buf.data);
    memcpy(here,&buf,sizeof(aesblock));
    here++;
  }
}
class YsDB{
  public:
  leveldb::DB *              user;
  leveldb::DB *              userTime;
  leveldb::DB *              userTmp;
  std::map<int32_t,location> userIP;
  leveldb::DB *              sourceBoardcast;
  leveldb::DB *              sourceUser;
  leveldb::DB *              keys;
  std::list<location>        livelist;
  leveldb::DB *              unique;
  RWMutex                    locker;
  RWMutex                    livelocker;
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
*[4]        live mode           t,f
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
    assert(leveldb::DB::Open(options, "data/unique", &unique).ok());
  }
  ~YsDB(){
    delete user;
    delete userTime;
    delete userTmp;
    delete sourceBoardcast;
    delete sourceUser;
    delete keys;
    delete unique;
  }
  void liveAdd(location & address){
    livelocker.Wlock();
    livelist.push_back(address);
    livelocker.unlock();
  }
  void liveClean(){
    livelocker.Wlock();
    livelist.clear();
    livelocker.unlock();
  }
  void live(void(*callback)(location &,void*),void * arg){
    livelocker.Rlock();
    std::list<location>::iterator it;
    for(it=livelist.begin();it!=livelist.end();it++){
      callback(*it,arg);
    }
    livelocker.unlock();
  }
  bool getkey(int32_t uid,aesblock * key){
    char name[9];
    std::string v;
    int2str(&uid,name);
    if(!keys->Get(leveldb::ReadOptions(),name,&v).ok())return 0;
    if(v.length()<16)return 0;
    for(int i=0;i<16;i++) key->data[i]=v[i];
    return 1;
  }
  void setkey(int32_t uid,aesblock * key){
    char name[9];
    int2str(&uid,name);
    char v[17];
    for(int i=0;i<16;i++)v[i]=key->data[i];
    v[16]='\0';
    keys->Put(leveldb::WriteOptions(),name,v);
  }
  bool logunique(int32_t uid,int32_t lid){
    if(lid==0)return 1;
    char keyname[256];
    char timestr[64];
    std::string v;
    sprintf(keyname,"%d:%d",uid,lid);
    if(unique->Get(leveldb::ReadOptions(),keyname,&v).ok())
      if(!v.empty())return 0;
    sprintf(timestr,"%d",nowtime_s());
    unique->Put(leveldb::WriteOptions(),keyname,timestr);
    return 1;
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
    locker.Wlock();
    location p;
    p.ip=ip;
    p.port=port;
    userIP[userid]=p;
    locker.unlock();
  }
  bool getUserIP(int32_t userid,in_addr *ip,short *port){
    locker.Rlock();
    auto pt=userIP.find(userid);
    if(pt==userIP.end()){
      *port=0;
      ip->s_addr=0;
      locker.unlock();
      return 0;
    }
    location &p=pt->second;
    *ip=p.ip;
    *port=p.port;
    locker.unlock();
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
class ysConnection:public serverBase{
  public:
  const char   *       script;
  int32_t              myuserid;
  char                 mypassword[17];
  in_addr              parIP;
  short                parPort;
  std::stack<location> ipstack;
  std::mutex           iplocker;
  char                 globalmode;
  bool                 iscrypt;
  aesblock             key;
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
  virtual bool succeed(in_addr to,short port,int32_t uq){
    netQuery data;
    bzero(&data, sizeof(data));
    data.header.userid=myuserid;
    for(int i=0;i<16;i++)
      data.header.password[i]=mypassword[i];
    data.header.mode=_SUCCEED;
    return send(to,port,&data,sizeof(data));
  }
  virtual bool fail(in_addr to,short port,int32_t uq){
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
      self->succeed(ip,port,0);
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
      self->fail(ip,port,0);
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
        lua_pushinteger(L,buf.header.userid());
        //push password
        wristr(buf.header.password,bufs);
        lua_pushstring(L,bufs);
        //push querys
        wristr(buf.str1, bufs);
        lua_pushstring(L,bufs);
        wristr(buf.str2, bufs);
        lua_pushstring(L,bufs);
        lua_pushinteger(L,buf.num1());
        lua_pushinteger(L,buf.num2());
        lua_pushinteger(L,buf.num3());
        lua_pushinteger(L,buf.num4());
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
  virtual bool plus(in_addr from,short fport,void * data,std::string * u,netQuery * result){
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
    lua_pushinteger(lua,query->header.userid());
    lua_setglobal(lua,"USERID");
    wristr(query->header.function,buffer);
    lua_pushstring(lua,buffer);
    lua_setglobal(lua,"FUNCTION_NAME");
    lua_pushstring(lua,u->c_str());
    lua_setglobal(lua,"USERINFO");
    funcreg(lua);
    
    luaL_dofile(lua,script);
    
    lua_getglobal(lua,"NUM1");
    if(lua_isinteger(lua,-1)) result->num1=lua_tointeger(lua,-1);
    lua_pop(lua,1);
    
    lua_getglobal(lua,"NUM2");
    if(lua_isinteger(lua,-1)) result->num2=lua_tointeger(lua,-1);
    lua_pop(lua,1);
    
    lua_getglobal(lua,"NUM3");
    if(lua_isinteger(lua,-1)) result->num3=lua_tointeger(lua,-1);
    lua_pop(lua,1);
    
    lua_getglobal(lua,"NUM4");
    if(lua_isinteger(lua,-1)) result->num4=lua_tointeger(lua,-1);
    lua_pop(lua,1);
    
    int i;
    const char * ostr;
    
    lua_getglobal(lua,"str1");
    if(lua_isstring(lua,-1)){
      ostr=lua_tostring(lua,-1);
      for(i=0;(i<16 && ostr[i]!='\0');i++)
        result->str1[i]=ostr[i];
    }
    lua_pop(lua,1);
    
    lua_getglobal(lua,"str2");
    if(lua_isstring(lua,-1)){
      ostr=lua_tostring(lua,-1);
      for(i=0;(i<16 && ostr[i]!='\0');i++)
        result->str2[i]=ostr[i];
    }
    lua_pop(lua,1);
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
    bool crypt=0;
    aesblock key;
    if(header->crypt=='t'){
      crypt=1;
      if(ysDB.getkey(header->userid(),&key)){
        crypt_decode(source,&key);
      }
    }
    if(from.s_addr==parIP.s_addr)
    if(port==parPort){
      if(header->mode==_LOGIN){
        ysDB.newUser(header->userid());
        ysDB.changePwd(header->userid(),header->password);
        ysDB.changeAcl(header->userid(),"ttftffffffffffff");
        return 1;
      }else
      if(header->mode==_P2PCONNECT){
        iplocker.lock();
        if(from.s_addr==parIP.s_addr)
        if(port==parPort)
        if(p2pconnect(query->addr,query->num1())){
          location lc;
          lc.ip  =parIP;
          lc.port=parPort;
          parIP  =query->addr;
          parPort=query->num1();
          ipstack.push(lc);
          iplocker.unlock();
          return 1;
        }else{
          iplocker.unlock();
          return 0;
        }
      }else
      if(header->mode==_P2PCONNECT_C){
        p2pconnect(query->addr,query->num1());
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
      q.header.unique=header->unique;
      if(crypt)crypt_encode(&q,&key);
      send(from,port,&q,sizeof(q));
      return 1;
    }
    if(!ysDB.login(header->userid(),header->password,&userinfo)){
      if(!checkInParent(header->userid(),header->password)){
        fail(from,port,header->unique);
        return 0;
      }
    }
    if(!ysDB.logunique(header->userid(),header->unique)){
      //fail(from,port,header->unique);
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
    int32_t rvuserid=header->userid();
    int2str(&(rvuserid),name);
    int2str(&num,number);
    FILE * ff;
    //begin : 12
    //end   : 35
    Key senddata;
    aesblock newkey;
    switch(header->mode){
      case _UPDATEKEY:
        senddata=Key((Key::netSendkey*)&source->source,Key::read());
        senddata.buf=(Key::netSendkey*)&respk.source;
        senddata.setkey();
        for(i=0;i<16;i++) newkey.data[i]=senddata.key[i];
        ysDB.setkey(header->userid(),&newkey);
        respk.header.userid=myuserid;
        for(i=0;i<16;i++)
          respk.header.password[i]=mypassword[i];
        respk.size=sizeof(Key::netSendkey);
        
        respk.header.mode=_UPDATEKEY;
        
        respk.header.unique=header->unique;
        
        if(crypt)crypt_encode(&respk,&key);
        send(from,port,&respk,sizeof(respk));
        return 1;
      break;
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
              succeed(from,port,header->unique);
              createfile(filename);
              return 1;
            }
          }
        }else{
          if(!ysDB.existSrc(query->str1,header->userid()))
          if(ysDB.writeSrc(query->str1,header->userid(),filename)){
            succeed(from,port,header->unique);
            createfile(filename);
            return 1;
          }
        }
        fail(from,port,header->unique);
        return 0;
      break;
      case _SETSRC_APPEND:
        //get file name
        if((source->size())>SOURCE_CHUNK_SIZE || (source->size())<0){
          fail(from,port,header->unique);
          return 0;
        }
        if(header->globalMode=='t'){
          if(userinfo.at(16)=='t'){
            if(!ysDB.readSrc(source->title,&result)){
              fail(from,port,header->unique);
              return 0;
            }
          }
        }else{
          if(!ysDB.readSrc(source->title,header->userid(),&result)){
            fail(from,port,header->unique);
            return 0;
          }
        }
        filenamer=result.c_str();
        fileappend(filename,source->source,source->size());
        //get file name end
        succeed(from,port,header->unique);
        return 1;
      break;
      case _GETSRC:
        //get file name
        if(header->globalMode=='t'){
            if(!ysDB.readSrc(query->str1,&result)){
              fail(from,port,header->unique);
              return 0;
            }
        }else{
          if(!ysDB.readSrc(query->str1,header->userid(),&result)){
            fail(from,port,header->unique);
            return 0;
          }
        }
        filenamer=result.c_str();
        //get file name end
        bzero(&respk,sizeof(respk));
        ff=fopen(filenamer,"r");
        fseek(ff,SOURCE_CHUNK_SIZE*query->num1(),SEEK_SET);
        for(i=0;(i<SOURCE_CHUNK_SIZE && !feof(ff));i++){
          respk.source[i]=fgetc(ff);
        }
        fclose(ff);
        respk.header.userid=myuserid;
        for(i=0;i<16;i++)
          respk.header.password[i]=mypassword[i];
        
        respk.header.unique=header->unique;
        
        respk.size=i;
        respk.header.mode=_SETSRC_APPEND;
        wristr(query->str1,respk.title);
        if(crypt)crypt_encode(&respk,&key);
        send(from,port,&respk,sizeof(respk));
        return 1;
      break;
      case _GETSRCNAME:
        //get file name
        if(header->globalMode=='t'){
            if(!ysDB.readSrc(query->str1,&result)){
              fail(from,port,header->unique);
              return 0;
            }
        }else{
          if(!ysDB.readSrc(query->str1,header->userid(),&result)){
            fail(from,port,header->unique);
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
        
        respk.header.unique=header->unique;
        
        if(crypt)crypt_encode(&respk,&key);
        send(from,port,&respk,sizeof(respk));
        return 1;
      break;
      case _DELSRC:
        if(header->globalMode=='t'){
          if(userinfo.at(16)=='t'){
            if(ysDB.readSrc(query->str1,&result)){
                      ysDB.delSrc(query->str1);
            }else{
              fail(from,port,header->unique);
              return 0;
            }
          }
        }else{
          if(ysDB.readSrc(query->str1,header->userid(),&result)){
                    ysDB.delSrc(query->str1,header->userid());
          }else{
            fail(from,port,header->unique);
            return 0;
          }
        }
        if(remove(result.c_str())==-1){
          fail(from,port,header->unique);
          return 0;
        }
        succeed(from,port,header->unique);
        return 1;
      break;
      case _NEWUSER:
        if(userinfo.at(17)!='t'){
          fail(from,port,header->unique);
          return 0;
        }
        ysDB.newUser(query->num1());
        succeed(from,port,header->unique);
        return 1;
      break;
      case _DELUSER:
        if(userinfo.at(17)!='t'){
          fail(from,port,header->unique);
          return 0;
        }
        ysDB.delUser(query->num1());
        succeed(from,port,header->unique);
        return 1;
      break;
      case _SETACL:
        if(userinfo.at(17)!='t'){
          fail(from,port,header->unique);
          return 0;
        }
        if(ysDB.changeAcl(query->num1(),query->str1)){
          succeed(from,port,header->unique);
          return 1;
        }else{
          fail(from,port,header->unique);
          return 0;
        }
      break;
      case _SETPWD:
        if(header->globalMode=='t'){
          if(userinfo.at(17)!='t'){
            fail(from,port,header->unique);
            return 0;
          }
          if(ysDB.changePwd(query->num1(),query->str1)){
            succeed(from,port,header->unique);
            return 1;
          }else{
            fail(from,port,header->unique);
            return 0;
          }
        }else{
          if(!ysDB.user->Get(leveldb::ReadOptions(),name,&result).ok()){fail(from,port,header->unique);return 0;}
          if(result.empty()){fail(from,port,header->unique);return 0;}
          tp=result.c_str();
          for(int i=0;i<16;i++){
            if(query->str2[i]=='\0') {fail(from,port,header->unique);return 0;}
            if(         tp[i]=='\0') {fail(from,port,header->unique);return 0;}
            if(query->str2[i]!=tp[i]){fail(from,port,header->unique);return 0;}
          }
          if(ysDB.changePwd(header->userid(),query->str1)){
            succeed(from,port,header->unique);
            return 1;
          }else{
            fail(from,port,header->unique);
            return 0;
          }
        }
      break;
      case _LOGIN:
        if(ysDB.setTmpPwd(header->userid(),query->str1)){
          succeed(from,port,header->unique);
          return 1;
        }else{
          fail(from,port,header->unique);
          return 0;
        }
      break;
      case _LOGIP:
        if(userinfo.at(18)!='t'){
          fail(from,port,header->unique);
          return 0;
        }  
        ysDB.logIP(header->userid(),from,port);
        succeed(from,port,header->unique);
        return 1;
      break;
      case _LIVESRC:
        if(userinfo.at(20)!='t'){
          fail(from,port,header->unique);
          return 0;
        }
        
        bzero(&respk,sizeof(respk));
        
        for(i=0;i<SOURCE_CHUNK_SIZE;i++)
          respk.source[i]=source->source[i];
        
        respk.header.userid=myuserid;
        for(i=0;i<16;i++)
          respk.header.password[i]=mypassword[i];
        
        respk.header.unique=header->unique;
        
        respk.size=source->size;
        respk.header.mode=_LIVE;
        respk.header.crypt='f';
        
        wristr(source->title,respk.title);
        
        for(std::list<location>::iterator it=ysDB.livelist.begin();it!=ysDB.livelist.end();it++){
          for(i=0;i<4;i++)
            send(it->ip,it->port,&respk,sizeof(respk));
        }
        
      break;
      case _CONNECTUSER:
        connectUser(query->num1(),from,port);
        return 1;
      break;
      case _SHELL:
        if(userinfo.at(19)!='t'){
          fail(from,port,header->unique);
          return 0;
        }
        system(source->source);
        succeed(from,port,header->unique);
        return 1;
      break;
      case _PLUS:
        bzero(&qypk,sizeof(qypk));
        plus(from,port,data,&userinfo,&qypk);
        qypk.header.unique=header->unique;
        
        if(crypt)crypt_encode(&qypk,&key);
        
        send(from,port,&qypk,sizeof(qypk));
        
        return 1;
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
    netQuery  qypk;
    netSource buf;
    int      i,rdn;
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
    
    rdn=randnum();
    qypk.header.unique=rdn;
    
    if(iscrypt)crypt_encode(&qypk,&key);
    
    for(i=0;i<10;i++){
      send(parIP,parPort,&qypk,sizeof(qypk));
      loginloop1:
      if(recv_within_time(&from,&port,&buf,sizeof(buf),1,0)){
        if(from.s_addr==parIP.s_addr && port==parPort){
          crypt_decode(&qypk,&key);
          
          if(rdn!=qypk.header.unique) goto loginloop1;
          
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
    
    bzero(&qypk,sizeof(qypk));
    bzero(&buf ,sizeof(buf ));
    qypk.header.mode=_GETSERVERINFO;
    qypk.header.userid=myuserid;
    wristr(mypassword,qypk.header.password);
    
    rdn=randnum();
    qypk.header.unique=rdn;
    
    if(iscrypt)crypt_encode(&qypk,&key);
    
    for(i=0;i<10;i++){
      send(parIP,parPort,&qypk,sizeof(qypk));
      loginloop2:
      if(recv_within_time(&from,&port,&buf,sizeof(buf),1,0)){
        if(from.s_addr==parIP.s_addr && port==parPort){
          crypt_decode(&qypk,&key);
          
          if(rdn!=qypk.header.unique) goto loginloop2;
          
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
      //lwan_status_debug("free a block");
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
      lwan_status_debug("mallocCount=%d",mallocCount);
      lwan_status_debug("usingCount=%d",usingCount);
      lwan_status_debug("free pool");
    }
    require * get(){
      locker.lock();
      //lwan_status_debug("get chunk");
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
      //lwan_status_debug("del chunk");
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
    lwan_status_debug("\033[40;43mYRSSF:\033[0mserver shutdown success\n");
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
    std::cout << "\033[40;43mYRSSF:\033[0mserver running on \0"<<ntohs(server_addr.sin_port)<< std::endl;
    r=pool.get();
    r->self=this;
    while(isrunning){
      if(!wait_for_data(1,0))continue;
      if(recv(&(r->from),&(r->port),r->buffer,sizeof(netSource))){
        if(pthread_create(&newthread,NULL,accept_req,r)!=0){
          perror("pthread_create");
        }
        r=pool.get();
        r->self=this;
      }
    }
    pool.del(r);
    w.unlock();
  }
}server(SERVER_PORT);
class Client:public Server{
  public:
  bool liveclientrunning;
  Client(short p):Server(p){
    globalmode='f';
    iscrypt=0;
    script="client.lua";
    liveclientrunning=0;
  }
  void live(const char * data,uint32_t size){
    netSource qypk;
    uint32_t  len;
    
    qypk.header.mode=_LIVESRC;
    qypk.header.userid=myuserid;
    
    int rdn=randnum();
    qypk.header.unique=rdn;
    
    len = size>SOURCE_CHUNK_SIZE ? SOURCE_CHUNK_SIZE : size;
    qypk.size=len;
    
    int i;
    for(i=0;i<len;i++) qypk.source[i]=data[i];
    
    wristr(mypassword,qypk.header.password);
    
    if(iscrypt)crypt_encode(&qypk,&key);
    
    for(i=0;i<8;i++)
      send(parIP,parPort,&qypk,sizeof(qypk));
  }
  void live(netSource * qypk){
    uint32_t  len;
    int i;
    
    qypk->header.mode=_LIVESRC;
    qypk->header.userid=myuserid;
    
    int rdn=randnum();
    qypk->header.unique=rdn;
    
    wristr(mypassword,qypk->header.password);
    
    if(iscrypt)crypt_encode(qypk,&key);
    
    for(i=0;i<8;i++)
      send(parIP,parPort,&qypk,sizeof(qypk));
  }
  void liveclient(bool(*callback)(void*,int,void*),void * arg){
    netSource buf;
    in_addr   from;
    short     port;
    while(liveclientrunning){
      if(!wait_for_data(1,0))continue;
      bzero(&buf,sizeof(buf));
      if(recv(&from,&port,&buf,sizeof(netSource))){
        if(from.s_addr==parIP.s_addr && port==parPort){
          if(!ysDB.logunique(buf.header.userid(),buf.header.unique)) continue;
          if(buf.header.mode!=_LIVE)continue;
          if(!callback(&(buf.source),buf.size(),arg))return;
        }
      }
    }
  }
  bool updatekey(){
    netSource qypk;
    netSource buf;
    int i;
    in_addr  from;
    short    port;
    bzero(&qypk,sizeof(qypk));
    bzero(&buf ,sizeof(buf ));
    Key senddata((Key::netSendkey*)&(qypk.source));
    qypk.header.mode=_UPDATEKEY;
    qypk.header.userid=myuserid;
    
    int rdn=randnum();
    qypk.header.unique=rdn;
    
    qypk.size=sizeof(Key::netSendkey);
    wristr(mypassword,qypk.header.password);
    if(iscrypt)crypt_encode(&qypk,&key);
    for(i=0;i<10;i++){
      send(parIP,parPort,&qypk,sizeof(qypk));
      uploop2:
      if(recv_within_time(&from,&port,&buf,sizeof(buf),1,0)){
        if(from.s_addr==parIP.s_addr && port==parPort){
          crypt_decode(&buf,&key);
          
          if(rdn!=qypk.header.unique) goto uploop2;
          
          Key ded((Key::netSendkey*)&(buf.source),senddata.privatekey);
          for(i=0;i<16;i++){
            this->key.data[i]=ded.key[i];
          }
          return 1;
        }
        else
          goto uploop2;
      }
    }
    return 0;
  }
  bool connectToUser(int32_t uid,in_addr * oaddr,short * oport){
    netQuery qypk;
    netQuery buf;
    int      i;
    in_addr  from;
    short    port;
    bzero(&qypk,sizeof(qypk));
    bzero(&buf ,sizeof(buf ));
    qypk.header.mode=_CONNECTUSER;
    wristr(mypassword,qypk.header.password);
    
    int rdn=randnum();
    qypk.header.unique=rdn;
    
    qypk.header.userid=myuserid;
    qypk.header.globalMode=globalmode;
    qypk.num1=uid;
    if(iscrypt)crypt_encode(&qypk,&key);
    for(i=0;i<10;i++){
      send(parIP,parPort,&qypk,sizeof(qypk));
      dsloop1:
      if(recv_within_time(&from,&port,&buf,sizeof(buf),1,0)){
        if(from.s_addr==parIP.s_addr && port==parPort){
          crypt_decode(&buf,&key);
          if(buf.header.mode==_P2PCONNECT){
            *oport =buf.num1();
            *oaddr =buf.addr;
            if(p2pconnect(buf.addr,buf.num1()))
              return 1;
            else
              return 0;
          }else
            return 0;
        }
        else
          goto dsloop1;
      }
    }
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
    
    int rdn=randnum();
    qypk.header.unique=rdn;
    
    qypk.header.userid=myuserid;
    qypk.header.globalMode=globalmode;
    if(iscrypt)crypt_encode(&qypk,&key);
    for(i=0;i<10;i++){
      send(parIP,parPort,&qypk,sizeof(qypk));
      dsloop1:
      if(recv_within_time(&from,&port,&buf,sizeof(buf),1,0)){
        if(from.s_addr==parIP.s_addr && port==parPort){
          crypt_decode(&buf,&key);
          
          if(rdn!=qypk.header.unique) goto dsloop1;
          
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
    
    int rdn=randnum();
    qypk.header.unique=rdn;
    
    for(i=0;i<8;i++)
      qypk.str1[i]=sname[i];
    qypk.header.globalMode=globalmode;
    wristr(mypassword,qypk.header.password);
    if(iscrypt)crypt_encode(&qypk,&key);
    for(i=0;i<10;i++){
      send(parIP,parPort,&qypk,sizeof(qypk));
      dcloop1:
      if(recv_within_time(&from,&port,&buf,sizeof(buf),1,0)){
        if(from.s_addr==parIP.s_addr && port==parPort){
          crypt_decode(&buf,&key);
          
          if(rdn!=qypk.header.unique) goto dcloop1;
          
          if(buf.header.mode==_SETSRC_APPEND){
            int size;
            size=buf.size()>SOURCE_CHUNK_SIZE ? SOURCE_CHUNK_SIZE : buf.size();
            size=size>0 ? size : 0;
            fwrite(buf.source,size,1,path);
            if(buf.size()==SOURCE_CHUNK_SIZE)
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
    //int r=1;
    
    int rdn=randnum();
    qypk.header.unique=rdn;
    
    qypk.size=size;
    for(i=0;i<8;i++)
      qypk.title[i]=sname[i];
    qypk.header.globalMode=globalmode;
    wristr(mypassword,qypk.header.password);
    for(i=0;(i<size && i<SOURCE_CHUNK_SIZE);i++){
      qypk.source[i]=buf2[i];
    }
    if(iscrypt)crypt_encode(&qypk,&key);
    for(i=0;i<10;i++){
      send(parIP,parPort,&qypk,sizeof(qypk));
      ucloop2:
      if(recv_within_time(&from,&port,&buf,sizeof(buf),1,0)){
        if(from.s_addr==parIP.s_addr && port==parPort){
          crypt_decode(&buf,&key);
          
          if(rdn!=qypk.header.unique) goto ucloop2;
          
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
    
    int rdn=randnum();
    qypk.header.unique=rdn;
    
    qypk.header.userid=myuserid;
    qypk.header.globalMode=globalmode;
    if(iscrypt)crypt_encode(&qypk,&key);
    for(i=0;i<10;i++){
      send(parIP,parPort,&qypk,sizeof(qypk));
      dsloop1:
      if(recv_within_time(&from,&port,&buf,sizeof(buf),1,0)){
        if(from.s_addr==parIP.s_addr && port==parPort){
          crypt_decode(&buf,&key);
          
          if(rdn!=qypk.header.unique) goto dsloop1;
          
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
    
    int rdn=randnum();
    qypk.header.unique=rdn;
    
    qypk.header.userid=myuserid;
    if(iscrypt)crypt_encode(&qypk,&key);
    for(i=0;i<10;i++){
      send(parIP,parPort,&qypk,sizeof(qypk));
      dsloop1:
      if(recv_within_time(&from,&port,&buf,sizeof(buf),1,0)){
        if(from.s_addr==parIP.s_addr && port==parPort){
          crypt_decode(&buf,&key);
          
          if(rdn!=qypk.header.unique) goto dsloop1;
          
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
    
    int rdn=randnum();
    qypk.header.unique=rdn;
    
    qypk.header.userid=myuserid;
    qypk.header.globalMode='t';
    qypk.num1=puserid;
    if(iscrypt)crypt_encode(&qypk,&key);
    for(i=0;i<10;i++){
      send(parIP,parPort,&qypk,sizeof(qypk));
      dsloop1:
      if(recv_within_time(&from,&port,&buf,sizeof(buf),1,0)){
        if(from.s_addr==parIP.s_addr && port==parPort){
          crypt_decode(&buf,&key);
          
          if(rdn!=qypk.header.unique) goto dsloop1;
          
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
    
    int rdn=randnum();
    qypk.header.unique=rdn;
    
    qypk.header.userid=myuserid;
    qypk.header.globalMode='t';
    if(iscrypt)crypt_encode(&qypk,&key);
    for(i=0;i<10;i++){
      send(parIP,parPort,&qypk,sizeof(qypk));
      dsloop1:
      if(recv_within_time(&from,&port,&buf,sizeof(buf),1,0)){
        if(from.s_addr==parIP.s_addr && port==parPort){
          crypt_decode(&buf,&key);
          
          if(rdn!=qypk.header.unique) goto dsloop1;
          
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
sqlite3  * db;
std::mutex clientlocker;
bool clientdisabled=0;
int livefifo;
int liveserverfifo;
static bool runliveclient_cb(void * data,int size,void*){
  write(livefifo,data,size>SOURCE_CHUNK_SIZE?SOURCE_CHUNK_SIZE:size);
  return 1;
}
static void * runliveclient_th(void *){
  if(clientdisabled)return 0;
  clientdisabled=1;
  clientlocker.lock();
  client.liveclientrunning=1;
  lwan_status_debug("live mode on");
  client.liveclient(runliveclient_cb,NULL);
  lwan_status_debug("live mode off");
  clientlocker.unlock();
  clientdisabled=0;
}
static int runliveclient(){
  pthread_t newthread;
  if(pthread_create(&newthread,NULL,runliveclient_th,NULL)!=0)
    perror("pthread_create");
}
static void * liveserver_cb(void * arg){
  auto req=(require*)arg;
  auto bufm=(netSource*)(req->buffer);
  client.live(bufm);
  //lwan_status_debug("live size=%d",bufm->size());
  pool.del(req);
}
require *  liveserverreq;
static void * liveserver(void *){
  int lfd,len;
  pthread_t newthread;
  liveserverreq=pool.get();
  auto bufm=(netSource*)(liveserverreq->buffer);
  auto buf=bufm->source;
  while(1){
    lfd=open("live/server",O_RDONLY);
    lwan_status_debug("live begin");
    while(len=read(lfd,buf,SOURCE_CHUNK_SIZE)){
      bufm->size=len;
      if(pthread_create(&newthread,NULL,liveserver_cb,liveserverreq)!=0){
        //perror("pthread_create");
        liveserver_cb(liveserverreq);
      }
      liveserverreq=pool.get();
      bufm=(netSource*)(liveserverreq->buffer);
      buf=bufm->source;
    }
    lwan_status_debug("live end");
    close(lfd);
  }
}
namespace videolive{
  struct rgb565{
    unsigned short int pix565;
    void torgb(int * r,int * g,int * b){
      *r = ((pix565)>>11)&0x1f;
      *g = ((pix565)>>5)&0x3f;
      *b = (pix565)&0x1f;
    }
  };
  class shot{
    int fd;
    public:
    rgb565 * data;
    struct fb_var_screeninfo fb_var_info;
    struct fb_fix_screeninfo fb_fix_info;
    int buffer_size;
    shot(const char * path){
      fd = open(path, O_RDONLY);
      if(fd < 0){
      printf("can not open dev\n");
        exit(1);
      }
      // 获取LCD的可变参数
      ioctl(fd, FBIOGET_VSCREENINFO, &fb_var_info);
      // 一个像素多少位    
      printf("bits_per_pixel: %d\n", fb_var_info.bits_per_pixel);
      // x分辨率
      printf("xres: %d\n", fb_var_info.xres);
      // y分辨率
      printf("yres: %d\n", fb_var_info.yres);
      // r分量长度(bit)
      printf("red_length: %d\n", fb_var_info.red.length);
      // g分量长度(bit)
      printf("green_length: %d\n", fb_var_info.green.length);
      // b分量长度(bit)
      printf("blue_length: %d\n", fb_var_info.blue.length);
      // t(透明度)分量长度(bit)
      printf("transp_length: %d\n", fb_var_info.transp.length);
      // r分量偏移
      printf("red_offset: %d\n", fb_var_info.red.offset);
      // g分量偏移
      printf("green_offset: %d\n", fb_var_info.green.offset);
      // b分量偏移
      printf("blue_offset: %d\n", fb_var_info.blue.offset);
      // t分量偏移
      printf("transp_offset: %d\n", fb_var_info.transp.offset);

      // 获取LCD的固定参数
      ioctl(fd, FBIOGET_FSCREENINFO, &fb_fix_info);
      // 一帧大小
      printf("smem_len: %d\n", fb_fix_info.smem_len);
      // 一行大小
      printf("line_length: %d\n", fb_fix_info.line_length);
      // 一帧大小
      buffer_size = (fb_var_info.xres * fb_var_info.yres * fb_var_info.bits_per_pixel / 8);
      data = (rgb565 *)malloc(buffer_size);
    }
    ~shot(){
      if(fd>=0)close(fd);
      free(data);
    }
    int readbuffer(){
      return read(fd, data, buffer_size);
    }
    rgb565 * getpix(int x,int y){
      if(x<0 || y<0 || x>fb_var_info.xres || y>fb_var_info.yres) return NULL;
      return &data[x+y*fb_var_info.xres];
    }
  };
  struct pixel{
    uint8_t R,G,B;
    pixel(){}
    pixel(uint8_t r,uint8_t g,uint8_t b){
      R=r;
      G=g;
      B=b;
    }
  };
  struct netPack{
    char m;
    nint32 x,y,w,h;
    pixel data[];
  };
  class boardcast{
    shot shotbuf;
    public:
    float resizex,resizey;
    pixel buffer[600][500];
    boardcast(const char * path):shotbuf(path),resizex(1),resizey(1){
      resizex=shotbuf.fb_var_info.xres/600.0f;
      resizey=shotbuf.fb_var_info.yres/500.0f;
    }
    ~boardcast(){
    }
    void setpixel(pixel * px,int x,int y){
      if(x<0 || y<0 || x>=600 || y>=500) return;
      buffer[x][y].R=px->R;
      buffer[x][y].G=px->G;
      buffer[x][y].B=px->B;
    }
    void sendall(){
      int ix,iy;
      netSource nbuf;
      char * buf=nbuf.source;
      nbuf.size=SOURCE_CHUNK_SIZE;
      void * endp=&(buf[SOURCE_CHUNK_SIZE]);
      netPack * bufp=(netPack*)buf;
      pixel * pxl;
      //printf("send a chunk\n");
      for(iy=0;iy<(500/2);iy++){
        pxl=&(bufp->data[0]);
        bufp->x=0;
        bufp->y=iy*2;
        bufp->w=600;
        bufp->h=2;
        for(ix=0;ix<600;ix++){
          //line:iy*2
          pxl->R=buffer[ix][iy*2].R;
          pxl->G=buffer[ix][iy*2].G;
          pxl->B=buffer[ix][iy*2].B;
          pxl++;
        }
        client.live(&nbuf);
        for(ix=0;ix<600;ix++){
          //line:iy*2+1
          pxl->R=buffer[ix][iy*2+1].R;
          pxl->G=buffer[ix][iy*2+1].G;
          pxl->B=buffer[ix][iy*2+1].B;
          pxl++;
        }
        client.live(&nbuf);
      }
    }
    void sendshot(){
      shotbuf.readbuffer();
      rgb565 * pix;
      int r,g,b;
      //printf("send a frame\n");
      for(int ix=0;ix<600;ix++)
      for(int iy=0;iy<500;iy++){
        pix=shotbuf.getpix(ix*resizex,iy*resizey);
        if(pix==NULL){
          pix->torgb(&r,&g,&b);
          buffer[ix][iy].R=r;
          buffer[ix][iy].G=g;
          buffer[ix][iy].B=b;
        }
      }
      sendall();
    }
  };
  class live_f{
    boardcast * bd;
    public:
    live_f(){
      bd=NULL;
    }
    ~live_f(){
      if(bd)delete bd;
    }
    void init(const char * path){
      bd=new boardcast(path);
    }
    void sendall(){
      if(!bd)return;
      bd->sendall();
    }
  }screen;
}
class API{
  int shmid;
  public:
  API(){
    db=NULL;
    lwan_status_debug("\033[40;43mYRSSF:\033[0mdatabase loading...\n");
    int ret = sqlite3_open("./data/yrssf.db", &db);
    if( ret != SQLITE_OK ) {
      lwan_status_debug("can not open %s \n", sqlite3_errmsg(db));
      return;
    }
    signal(10,[](int){
      client.liveclientrunning=1;
      runliveclient();
    });
    signal(12,[](int){
      client.liveclientrunning=0;
      clientdisabled=0;
    });
    mkdir("./live",0777);
    livefifo=mkfifo("./live/client",0666);
    liveserverfifo=mkfifo("./live/server",0666);
  }
  ~API(){
    if(db)sqlite3_close(db);
    close(livefifo);
    close(liveserverfifo);
    pool.del(liveserverreq);
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
      if(clientdisabled)return 0;
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
      clientlocker.lock();
      client.upload(str,lua_tostring(L,2));
      clientlocker.unlock();
      return 0;
  }
  static int lua_download(lua_State * L){
      if(clientdisabled)return 0;
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
      clientlocker.lock();
      client.download(str,lua_tostring(L,2));
      clientlocker.unlock();
      return 0;
  }
  static int lua_del(lua_State * L){
      if(clientdisabled)return 0;
      char str[9];
      str[8]='\0';
      if(!lua_isstring(L,1))return 0;
      const char * sname=lua_tostring(L,1);
      for(int i=0;i<8;i++){
        str[i]=sname[i];
        if(sname[i]=='\0')
          break;
      }
      clientlocker.lock();
      client.del(str);
      clientlocker.unlock();
      return 0;
  }
  static int lua_setPwd(lua_State * L){
      if(clientdisabled)return 0;
      if(lua_isstring(L,1)){
        if(!lua_isstring(L,2))return 0;
        clientlocker.lock();
        client.setPwd(lua_tostring(L,1),lua_tostring(L,2));
        clientlocker.unlock();
        return 0;
      }else
      if(lua_isinteger(L,1)){
        if(!lua_isstring(L,2))return 0;
        clientlocker.lock();
        client.setPwd(lua_tostring(L,2),(int32_t)lua_tointeger(L,1));
        clientlocker.unlock();
        return 0;
      }
  }
  static int lua_newUser(lua_State * L){
      if(clientdisabled)return 0;
      if(!lua_isinteger(L,1))return 0;
      clientlocker.lock();
      client.newUser(lua_tointeger(L,1));
      clientlocker.unlock();
      return 0;
  }
  static int lua_cps(lua_State * L){//change parent server
      if(clientdisabled)return 0;
      if(!lua_isstring (L,1))return 0;
      if(!lua_isinteger(L,2))return 0;
      in_addr addr;
      addr.s_addr=inet_addr(lua_tostring(L,1));
      clientlocker.lock();
      lua_pushboolean(L,client.changeParentServer(addr,lua_tointeger(L,2)));
      clientlocker.unlock();
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
      if(clientdisabled)return 0;
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
  static int lua_connectToUser(lua_State * L){
    if(clientdisabled)return 0;
    in_addr addr;
    short   port;
    if(!lua_isinteger(L,1))return 0;
    clientlocker.lock();
    lua_pushboolean(L,(client.connectToUser(lua_tointeger(L,1),&addr,&port)));
    clientlocker.unlock();
    lua_pushstring(L,inet_ntoa(addr));
    lua_pushinteger(L,port);
    return 3;
  }
  static int lua_setkey(lua_State * L){
    if(clientdisabled)return 0;
    aesblock key;
    bzero(&key,sizeof(key));
    const char * str;
    if(!lua_isstring(L,1))return 0;
    str=lua_tostring(L,1);
    for(int i=0;(i<16 && str[i]);i++)
      key.data[i]=str[i];
    if(!lua_isinteger(L,2)){
      client.key=key;
    }else{
      ysDB.setkey(lua_tointeger(L,2),&key);
    }
    return 0;
  }
  static int lua_getkey(lua_State * L){
    aesblock key;
    char str[17];
    str[16]='\0';
    int i;
    aesblock *tk;
    if(!lua_isinteger(L,1)){
      tk=&(client.key);
    }else{
      tk=&key;
      ysDB.getkey(lua_tointeger(L,1),&key);
    }
    for(i=0;i<16;i++) str[i]=tk->data[i];
    lua_pushstring(L,str);
    return 1;
  }
  void funcreg(lua_State * L){
    lua_register(L,"clientUpload",        lua_upload);
    lua_register(L,"clientDownload",      lua_download);
    lua_register(L,"clientDel",           lua_del);
    lua_register(L,"clientSetPwd",        lua_setPwd);
    lua_register(L,"clientNewUser",       lua_newUser);
    lua_register(L,"changeParentServer",  lua_cps);
    lua_register(L,"connectUser",         lua_connectUser);
    lua_register(L,"connectToUser",       lua_connectToUser);
    lua_register(L,"getkey",              lua_getkey);
    lua_register(L,"setkey",              lua_setkey);
    lua_register(L,"runsql",              runsql);
    lua_register(L,"loadsharedlibs",[](lua_State * L){
      if(!lua_isstring(L,1))return 0;
      libs.open(lua_tostring(L,1));
      return 0;
    });
    lua_register(L,"boardcastScreenInit",[](lua_State * L){
      if(!lua_isstring(L,1))return 0;
      videolive::screen.init(lua_tostring(L,1));
    });
    lua_register(L,"boardcastScreen",[](lua_State * L){
      videolive::screen.sendall();
      return 0;
    });
    lua_register(L,"canQuery",[](lua_State * L){
      if(clientdisabled){
        lua_pushboolean(L,0);
        return 1;
      }
      if(clientlocker.try_lock()){
        clientlocker.unlock();
        lua_pushboolean(L,1);
      }else{
        lua_pushboolean(L,0);
      }
      return 1;
    });
    lua_register(L,"updatekey",[](lua_State * L){
      if(clientdisabled)return 0;
      clientlocker.lock();
      lua_pushboolean(L,client.updatekey());
      clientlocker.unlock();
      return 1;
    });
    lua_register(L,"globalModeOn",[](lua_State * L){
      client.globalmode='t';
      return 0;
    });
    lua_register(L,"globalModeOff",[](lua_State * L){
      client.globalmode='f';
      return 0;
    });
    lua_register(L,"cryptModeOn",[](lua_State * L){
      client.iscrypt=1;
      return 0;
    });
    lua_register(L,"cryptModeOff",[](lua_State * L){
      client.iscrypt=0;
      return 0;
    });
    lua_register(L,"liveadd",[](lua_State * L){
      location ad;
      if(!lua_isstring(L,1))return 0;
      ad.ip.s_addr=inet_addr(lua_tostring(L,1));
      if(!lua_isinteger(L,2))return 0;
      ad.port=lua_tointeger(L,2);
      ysDB.liveAdd(ad);
      return 0;
    });
    lua_register(L,"liveclean",[](lua_State * L){
      ysDB.liveClean();
      return 0;
    });
    lua_register(L,"liveModeOn",[](lua_State * L){
      if(clientdisabled)return 0;
      client.liveclientrunning=1;
      runliveclient();
      return 0;
    });
    lua_register(L,"liveModeOff",[](lua_State * L){
      client.liveclientrunning=0;
      clientdisabled=0;
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
    lwan_status_debug("\033[40;43mYRSSF:\033[0m");
    lwan_status_debug("\033[40;36mYRSSF create thread\033[0m\n");
    if(pthread_create(&newthread,NULL,runServerThread,NULL)!=0)
      perror("pthread_create");
  }
  Init(){
    pthread_t newthread;
    if(pthread_create(&newthread,NULL,liveserver,NULL)!=0)
      perror("pthread_create");
    lwan_status_debug("\033[40;43mYRSSF:\033[0m\033[40;36mYRSSF init\033[0m\n");
    gblua=luaL_newstate();
    luaL_openlibs(gblua);
    lua_pushinteger(gblua,(int)&server);
    lua_setglobal(gblua,"SERVER");
    lua_pushinteger(gblua,(int)&client);
    lua_setglobal(gblua,"CLIENT");
    ysConnection::funcreg(gblua);
    api.funcreg(gblua);
    luaL_dofile(gblua,"init.lua");
  }
  ~Init(){
    lwan_status_debug("\033[40;43mYRSSF:\033[0m\033[40;36mYRSSF server shutdown\033[0m\n");
    server.shutdown();
    lua_close(gblua);
  }
}init;
class Libloader{
  public:
  Libloader(){}
  ~Libloader(){}
  void load(const char * path){}
}libloader;
///////////////////////////
}//////////////////////////
///////////////////////////
extern "C" void runServer(){
    yrssf::init.run();
}
extern "C" void shutdownServer(){
    lwan_status_debug("shutdown");
    //yrssf::init.~Init();
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
    bzero(default_map,sizeof(default_map));
    default_map[0].prefix = "/ajax";
    default_map[0].handler = ajax;
    default_map[1].prefix = "/";
    default_map[1].module = lwan_module_serve_files();
    default_map[1].flags = (lwan_handler_flags_t)0;
    struct lwan_serve_files_settings_t servefile;
    default_map[1].args = &servefile;
    bzero(&servefile,sizeof(servefile));
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
    lwan_status_debug("\033[40;43mYRSSF:\033[0m\033[40;37mYRSSF shutdown\033[0m\n");
    
    default_map[0].prefix  = NULL;
    default_map[0].handler = NULL;
    default_map[1].prefix  = NULL;
    default_map[1].module  = NULL;
    default_map[1].flags   = (lwan_handler_flags_t)0;
    default_map[1].args    = NULL;
    
    shutdownServer();
    lwan_status_debug("\033[40;43mYRSSF:\033[0m\033[40;37mweb server shutdown\033[0m\n");
    lwan_shutdown(&l);
    lwan_status_debug("\033[40;43mYRSSF:\033[0m\033[40;37mweb shutdown success\033[0m\n");
    return 0;
}
class Automanager{
  static void * freeunique(void*){
    while(1){
      sleep(60);
      int time;
      int nowtime=yrssf::nowtime_s();
      int num=0;
      leveldb::ReadOptions options;
      options.snapshot = yrssf::ysDB.unique->GetSnapshot();
      leveldb::Iterator* it = yrssf::ysDB.unique->NewIterator(options);
      for(it->SeekToFirst(); it->Valid(); it->Next()){
        time=atoi(it->value().ToString().c_str());
        if(abs(time-nowtime)<1000) continue;
        yrssf::ysDB.unique->Delete(leveldb::WriteOptions(),it->key().ToString());
        num++;
      }
      delete it;
      yrssf::ysDB.unique->ReleaseSnapshot(options.snapshot);
      lwan_status_debug("free %d unique id",num);
    }
  }
  static void * freemems(void*){
    int num[24];
    int times[24];
    int nownum;
    int nowhour;
    int buf;
    int i;
    time_t timer;
    bzero(num,   sizeof(int)*24);
    bzero(times, sizeof(int)*24);
    while(1){
      sleep(3600);
      timer=time(NULL);
      struct tm *ptr  =localtime(&timer);
      nowhour         =ptr->tm_hour;
      nownum          =num[nowhour];
      buf             =num[nowhour]*times[nowhour];
      times[nowhour]++;
      num[nowhour]    =(buf+yrssf::pool.mallocCount)/times[nowhour];
      if(nownum<64) continue;
      if(abs((float)yrssf::pool.usingCount/(float)yrssf::pool.mallocCount)>0.8f) continue;
      i=0;
      while(yrssf::pool.mallocCount>(nownum+32)){
        yrssf::pool.freeone();
        i++;
      }
      lwan_status_debug("free %d memory block",i);
    }
  }
  public:
  Automanager(){
    pthread_t newthread;
    if(pthread_create(&newthread,NULL,freeunique,NULL)!=0)
      perror("pthread_create");
    if(pthread_create(&newthread,NULL,freemems,NULL)!=0)
      perror("pthread_create");
  }
  ~Automanager(){
  }
}automanager;