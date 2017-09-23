#ifndef yrssf_core_ysconnection
#define yrssf_core_ysconnection
#include "global.hpp"
#include <stack>
#include <map>
#include "server-base.hpp"
#include "classes.hpp"
#include "ysdb.hpp"
extern "C" {
#include "lua/src/lua.h"
#include "lua/src/lualib.h"
#include "lua/src/lauxlib.h"
#include "lua/src/luaconf.h"
}
#include "filecache.hpp"
namespace yrssf{
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
  char                 parpbk[ECDH_SIZE+1];
  std::string          parhash;
  ysConnection(short p):serverBase(p){
    mypassword[16]='\0';
    script="default.lua";
  }
  void fileappend(const char * n,char * data,int size){
    filecache::fileappend(n,data,size);
  }
  void createfile(const char *n){
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
    if(ipstack.empty()){
      iplocker.unlock();
      return 0;
      
    }
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
      if(!lua_isptr(L,3))return 0;
      auto self=(ysConnection*)lua_toptr(L,3);
      self->succeed(ip,port,0);
      return 0;
    });
    lua_register(lua,"fail",[](lua_State * L){
      if(!lua_isstring(L,1))return 0;
      in_addr ip;
      ip.s_addr=inet_addr(lua_tostring(L,1));
      if(!lua_isinteger(L,2))return 0;
      short port=lua_tointeger(L,2);
      if(!lua_isptr(L,3))return 0;
      auto self=(ysConnection*)lua_toptr(L,3);
      self->fail(ip,port,0);
      return 0;
    });
    lua_register(lua,"connect",[](lua_State * L){
      if(!lua_isstring(L,1))return 0;
      in_addr ip;
      ip.s_addr=inet_addr(lua_tostring(L,1));
      if(!lua_isinteger(L,2))return 0;
      short port=lua_tointeger(L,2);
      if(!lua_isptr(L,3))return 0;
      auto self=(ysConnection*)lua_toptr(L,3);
      lua_pushboolean(L,self->p2pconnect(ip,port));
      return 1;
    });
    lua_register(lua,"send",[](lua_State * L){
      if(!lua_isstring(L,1))return 0;
      in_addr ip;
      ip.s_addr=inet_addr(lua_tostring(L,1));
      if(!lua_isinteger(L,2))return 0;
      short port=lua_tointeger(L,2);
      if(!lua_isptr(L,3))return 0;
      auto self=(ysConnection*)lua_toptr(L,3);
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
        char ipbuf[32];
        inttoip(from,ipbuf);
        lua_pushstring (L,ipbuf);
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
    auto Lp=luapool::Create();
    lua=Lp->L;
    char ipbuf[32];
    inttoip(from,ipbuf);
    lua_pushstring (lua,ipbuf);
    lua_setglobal(lua,"FROM");
    lua_pushinteger(lua,port);
    lua_setglobal(lua,"FROM_PORT");
    lua_pushptr(lua,this);
    lua_setglobal(lua,"RUNNINGSERVER");
    lua_pushinteger(lua,query->header.userid());
    lua_setglobal(lua,"USERID");
    wristr(query->header.function,buffer);
    buffer[16]='\0';
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
    
    lua_getglobal(lua,"STR1");
    if(lua_isstring(lua,-1)){
      ostr=lua_tostring(lua,-1);
      for(i=0;(i<16 && ostr[i]!='\0');i++)
        result->str1[i]=ostr[i];
    }
    lua_pop(lua,1);
    
    lua_getglobal(lua,"STR2");
    if(lua_isstring(lua,-1)){
      ostr=lua_tostring(lua,-1);
      for(i=0;(i<16 && ostr[i]!='\0');i++)
        result->str2[i]=ostr[i];
    }
    lua_pop(lua,1);
    luapool::Delete(Lp);
  }
  virtual bool run(in_addr from,short port,void * data){
    std::string userinfo;
    netSource respk;
    netQuery  qypk;
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
    if(header->mode==_GET_PUBLIC_KEY){
      respk.header.crypt='f';
      respk.header.mode=_GET_PUBLIC_KEY;
      respk.header.unique=header->unique;
      respk.size=ECDH_SIZE;
      memcpy(
        DH.pubkey,
        respk.source,
        ECDH_SIZE
      );
      return 1;
    }
    if(header->mode==_REGISTER){
      ((netReg*)(&(source->source)))->run();
      succeed(from,port,header->unique);
      return 1;
    }
    if(from.s_addr==parIP.s_addr)
    if(port==parPort){
      /*
      *可能导致ip伪造攻击，于是取消
      *由_REGISTER代替
      if(header->mode==_LOGIN){
        ysDB.newUser(header->userid());
        ysDB.changePwd(header->userid(),header->password);
        ysDB.changeAcl(header->userid(),"ttftffffffffffff");
        return 1;
      }else
      */
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
        senddata=Key();
        senddata.buf=(Key::netSendkey*)&respk.source;
        senddata.initbuf();
        senddata.computekey((unsigned char*)&source->source);
        senddata.sign();
        
        for(i=0;i<16;i++) newkey.data[i]=senddata.shared[i];
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
            if(!ysDB.existSrc(query->str1)){
              if(ysDB.writeSrc(query->str1,filename)){
                succeed(from,port,header->unique);
                createfile(filename);
                return 1;
              }
            }else{
              ysDB.readSrc(query->str1,&result);
              createfile(result.c_str());
              succeed(from,port,header->unique);
              return 1;
            }
          }
        }else{
          if(!ysDB.existSrc(query->str1,header->userid())){
            if(ysDB.writeSrc(query->str1,header->userid(),filename)){
              succeed(from,port,header->unique);
              createfile(filename);
              return 1;
            }
          }else{
              ysDB.readSrc(query->str1,header->userid(),&result);
              createfile(result.c_str());
              succeed(from,port,header->unique);
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
        
        respk.size=filecache::readfile(
          filenamer,
          SOURCE_CHUNK_SIZE*query->num1(),
          SOURCE_CHUNK_SIZE,
          respk.source
        );
        
        respk.header.userid=myuserid;
        for(i=0;i<16;i++)
          respk.header.password[i]=mypassword[i];
        
        respk.header.unique=header->unique;
        
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
        if(filecache::removefile(result.c_str())==-1){
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
          if(!ysDB.ldata->Get(leveldb::ReadOptions(),userpre+name,&result).ok()){fail(from,port,header->unique);return 0;}
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
        
        livesrc(&respk,0);
        
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
        if(!config::AllowShell)return 0;
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
      case _CERT   :
        ((netReg*)&(respk.source))->init(
          header->userid(),
          header->password
        );
        
        respk.header.unique=header->unique;
        respk.header.mode=_CERT;
        
        if(crypt)crypt_encode(&respk,&key);
        
        send(from,port,&respk,sizeof(respk));
      break;
      case _LIVE_B:
        ysDB.liveAdd(location(from,port));
        succeed(from,port,header->unique);
      break;
      case _LIVE_E:
        ysDB.liveRemove(location(from,port));
        succeed(from,port,header->unique);
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
    //ysDebug("login");
    if(!getPbk())return 0;
    
    std::string dbkey="safe_key_";
    dbkey+=parhash;
    
    std::string dbv;
    //检查数据库中是否有aes密码
    if(ysDB.ldata->Get(leveldb::ReadOptions(),dbkey,&dbv).ok()){
      if(dbv.empty())goto no_key;//如果没有
      
      key.getbase64(dbv.c_str());
      iscrypt=1;
      //有，则启动加密
    }
    
    no_key:
    netQuery  qypk;
    netSource buf;
    int      i,rdn;
    in_addr  from;
    short    port;
    bzero(&qypk,sizeof(qypk));
    bzero(&buf ,sizeof(buf ));
    qypk.header.mode=_LOGIN;
    //ysDebug("copy password");
    wristr(mypassword,qypk.header.password);
    //ysDebug("copy userid");
    qypk.header.userid=myuserid;
    //ysDebug("copy temp pwd");
    wristr(randstr(),mypassword);
    //ysDebug("set temp pwd");
    wristr(mypassword,qypk.str1);
    //qypk.header.globalMode=globalmode;
    
    //ysDebug("query init");
    
    rdn=randnum();
    qypk.header.unique=rdn;
    
    if(iscrypt)crypt_encode(&qypk,&key);
    
    //ysDebug("send query");
    
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
    
    //ysDebug("get server info");
    
    bzero(&qypk,sizeof(qypk));
    bzero(&buf ,sizeof(buf ));
    qypk.header.mode=_GETSERVERINFO;
    qypk.header.userid=myuserid;
    wristr(mypassword,qypk.header.password);
    
    rdn=randnum();
    qypk.header.unique=rdn;
    
    if(iscrypt)crypt_encode(&qypk,&key);
    
    //ysDebug("send query");
    
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
  virtual void livesrc(netSource * respk,bool reconf){
    int i;
    if(reconf){
        
        respk->header.userid=myuserid;
        for(i=0;i<16;i++)
          respk->header.password[i]=mypassword[i];
        
        respk->header.unique=randnum();
        
        respk->header.mode=_LIVE;
        respk->header.crypt='f';
    }
    ysDB.livelocker.Rlock();
    for(auto it=ysDB.livelist.begin();it!=ysDB.livelist.end();it++){
      for(i=0;i<4;i++)
        send(it->ip,it->port,&respk,sizeof(respk));
    }
    ysDB.livelocker.unlock();
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
  bool getPbk(){
    netSource qypk;
    netSource buf;
    int i;
    in_addr  from;
    short    port;
    bzero(&qypk,sizeof(qypk));
    qypk.header.mode=_GET_PUBLIC_KEY;
    qypk.header.userid=myuserid;
    int rdn=randnum();
    qypk.header.unique=rdn;
    //wristr(mypassword,qypk.header.password);
    //获取公钥不需要密码
    if(iscrypt)crypt_encode(&qypk,&key);
    for(i=0;i<10;i++){
      send(parIP,parPort,&qypk,sizeof(qypk));
      dsloop1:
      if(recv_within_time(&from,&port,&buf,sizeof(buf),1,0)){
        if(from.s_addr==parIP.s_addr && port==parPort){
          crypt_decode(&buf,&key);
          
          if(rdn!=qypk.header.unique) goto dsloop1;
          
          if(buf.header.mode==_GET_PUBLIC_KEY){
            memcpy(
              parpbk,
              buf.source,
              ECDH_SIZE
            );
            parpbk[ECDH_SIZE]='\0';
            limonp::md5String(parpbk,parhash);
            //别慌，这是公钥，本来就没有保密的必要
            //所以用md5也无所谓了。
            //如果确实担心，请把这个换成sha（openssl里面有）
            return 1;
          }else{
            return 0;
          }
        }
        else
          goto dsloop1;
      }
    }
    return 0;
  }
  bool updatekey(){
    if(!getPbk())return 0;
    netSource qypk;
    netSource buf;
    int i;
    in_addr  from;
    short    port;
    bzero(&qypk,sizeof(qypk));
    bzero(&buf ,sizeof(buf ));
    Key senddata;
    senddata.buf=(Key::netSendkey*)&(qypk.source);
    senddata.initbuf();
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
          
          if(config::checkSign)
          if(!senddata.checksign())return 0;
          senddata.computekey((unsigned char*)&buf.source);
          for(i=0;i<16;i++){
            this->key.data[i] =senddata.shared[i];
          }
          //将密码保存至数据库
          std::string dbkey="safe_key_";
          dbkey+=parhash;
          std::string dbpwd=this->key.tobase64();
          ysDB.ldata->Put(leveldb::WriteOptions(),dbkey,dbpwd);
          
          return 1;
        }
        else
          goto uploop2;
      }
    }
    return 0;
  }
};
}
#endif