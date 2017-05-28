#ifndef yrssf_httpd_paser
#define yrssf_httpd_paser
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <memory.h>
#include <map>
#include <string>
#include "global.hpp"
namespace yrssf{
  namespace httpd{
    class requestBase{
      bool inited;
      public:
      int   fd;
      const char * url;
      const char * query;
      const char * cookie;
      bool  postmode;
      int   content_length;
      char  postdata[4096];
      //char  cookiedata[4096];
      std::map<std::string,std::string>
        paseredquery,
        paseredpost,
        paseredcookie,
        paseredheader;
      requestBase(){
        url=NULL;
        query=NULL;
        cookie=NULL;
        content_length=-1;
        inited=0;
      }
      ~requestBase(){
      }
      private:
      public:
      static char downchar(char in){//大小写转换
        if(in<'A')return in;
        if(in>'Z')return in;
        return (in+('a'-'A'));
      }
      static void downstr(char * in){
        auto inp=in;
        while(*inp){
          *inp=downchar(*inp);
          inp++;
        }
      }
      static void kv_paser(
        const char * str,
        char cl,
        char ce,
        std::map<std::string,std::string> & om
      ){
        //ysDebug("debug");
        int i;
        char buffer_k[128];
        char buffer_v[128];
        const char * cp;
        cp=str;
        while(*cp){
          for(i=0;(i<126 && (*cp)!=cl && cp!='\0');i++){
            buffer_k[i]=*cp;
            cp++;
          }
          buffer_k[i]='\0';
          cp++;
          for(i=0;(i<126 && (*cp)!=ce && cp!='\0');i++){
            buffer_v[i]=*cp;
            cp++;
          }
          buffer_v[i]='\0';
          cp++;
          om[buffer_k]=buffer_v;
        }
        //ysDebug("debug");
      }
      virtual void query_decode(){
        //ysDebug("debug");
        if(query==NULL)return;
        if(query[0]=='\0')return;
        //ysDebug("debug");
        kv_paser(query,'=','&',paseredquery);
      }
      virtual void cookie_decode(){
        if(cookie==NULL)return;
        if(cookie[0]=='\0')return;
        kv_paser(cookie,'=',';',paseredcookie);
      }
      virtual void post_decode(){
        postdata[sizeof(postdata)-1]='\0';
        if(postdata[0]=='\0')return;
        kv_paser(postdata,'=','&',paseredpost);
      }
      virtual void init(){
        if(inited)return;
        inited=1;
        readheader();
      }
      virtual bool getpost()=0;
      virtual bool getcookie()=0;
      virtual bool writePostIntoFile(int)=0;
      virtual void readheader()=0;
    };
  }
}
#endif
