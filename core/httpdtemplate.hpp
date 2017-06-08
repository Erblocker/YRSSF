#ifndef yrssf_httpd_template
#define yrssf_httpd_template
#include <sys/file.h>
#include <sys/socket.h>
#include <unistd.h>
#include <map>
#include <string>
#include <string.h>
#include "global.hpp"
namespace yrssf{
  namespace httpd{
  /*
  *example:
  * tpl T("hello.tpl");
  * T.callback=[](char c,void*){
  *   send(fd,&c,1);
  * };
  * T.assign["hello"]="world";
  * T.render();
  */
    class tpl{
      int fd;
      public:
      void (*callback)(char,void*);
      void *arg;
      std::map<std::string,std::string> assign;
      bool render(){
        if(callback==NULL)return 0;
        int len;
        char buf;
        char bufdata[33];
        char * bufp;
        char trash[32];
        while((len=read(fd,&buf,sizeof(buf)))>0){
          
          if(buf=='\0')return 1;
          if(buf=='<'){
            if(pread(fd,bufdata,32,lseek(fd,0,SEEK_CUR))>0){
              bufdata[32]='\0';
              //ysDebug("%s",bufdata);
              if(
                bufdata[0]=='!'&&
                bufdata[1]=='-'&&
                bufdata[2]=='-'&&
                bufdata[3]=='{'
              ){
                bufp=bufdata+4;
                bool ok=0;
                while(*bufp){
                  if(
                    bufp[0]=='}' &&
                    bufp[1]=='-' &&
                    bufp[2]=='-' &&
                    bufp[3]=='>'
                  ){
                    *bufp='\0';
                    ok=1;
                    break;
                  }
                  bufp++;
                }
                if(!ok){
                  callback(buf,arg);
                  continue;
                }
                bufp=bufdata+4;
                
                while(read(fd,&buf,1)>0){
                  if(buf=='>') break;
                }
                
                if(strlen(bufp)>0){
                  auto it=assign.find(bufp);
                  if(it==assign.end())continue;
                  auto strp=it->second.c_str();
                  while(*strp){
                    callback(*strp,arg);
                    strp++;
                  }
                  continue;
                }else{
                  continue;
                }
              }else{
                callback(buf,arg);
                continue;
              }
            }else{
              return 1;
            }
          }else{
            callback(buf,arg);
          }
        }
        return 1;
      }
      tpl(const char * file){
        fd=open(file,O_RDONLY);
      }
      ~tpl(){
        if(fd && fd!=-1)close(fd);
      }
    };
    int lua_template(lua_State * L){
      if(!lua_isinteger(L,1))return 0;
      if(!lua_isstring (L,2))return 0;
      if(!lua_istable  (L,3))return 0;
      
      struct sv{
        int fd;
      }s;
      s.fd=lua_tointeger(L,1);
      
      tpl T(lua_tostring(L,2));
      T.arg=&s;
      T.callback=[](char d,void * arg){
        auto s=(sv*)arg;
        send(s->fd,&d,1,0);
      };
      
      char bufk[32];
      
      int n = luaL_len(L,3);
      for(int i=1;i<=n;i++){
        lua_pushnumber(L,i);
        lua_gettable(L,3);
        if(!lua_istable(L,-1))return 0;
          
          lua_pushnumber(L,1);
          lua_gettable(L,-2);
          if(!lua_isstring(L,-1))return 0;
          const char * p=lua_tostring(L,-1);
          if(strlen(p)>30)return 0;
          strcpy(bufk,p);
          lua_pop(L,1);
          
          lua_pushnumber(L,2);
          lua_gettable(L,-2);
          if(!lua_isstring(L,-1))return 0;
          const char * v=lua_tostring(L,-1);
          lua_pop(L,1);
          
          T.assign[bufk]=v;
          
        lua_pop(L,1);
      }
      lua_pushboolean(L,T.render());
      return 1;
    }
  }
}
#endif