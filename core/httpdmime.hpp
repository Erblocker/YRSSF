#ifndef yrssf_httpd_mime
#define yrssf_httpd_mime
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <string>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "global.hpp"
namespace yrssf{
  namespace httpd{
    class Mime{
      std::map<std::string,std::string> mimelist;
      std::string getinfo(const char * path){
        std::string minfo;
        char buf[16];
        if(getext(path,buf)){
          auto it=mimelist.find(buf);
          if(it!=mimelist.end())
            return it->second;
        }
        return "text/plain";
      }
      public:
      bool getext(const char * path,char * buf){
        const char * tmp=strrchr(path,'.');
        if(tmp==NULL) return 0;
        tmp++;
        if((*tmp)=='\0')return 0;
        if(strlen(tmp)>14)return 0;
        strcpy(buf,tmp);
        return 1;
      }
      Mime(){
        mimelist["html"]	="text/html;charset=UTF-8";
        mimelist["txt"]		="text/plain;charset=UTF-8";
        mimelist["gif"]		="image/gif";
        mimelist["jpg"]		="image/jpeg";
        mimelist["mp2"]		="video/mpeg";
        mimelist["mp3"]		="audio/mpeg";
        mimelist["mpa"]		="video/mpeg";
        mimelist["mpe"]		="video/mpeg";
        mimelist["mpeg"]	="video/mpeg";
        mimelist["mpg"]		="video/mpeg";
        mimelist["svg"]		="image/svg+xml";
        mimelist["zip"]		="application/zip";
        mimelist["sh"]		="application/x-sh";
        mimelist["bmp"]		="image/bmp";
      }
      void setmime(const char * k,const char * v){
        mimelist[k]=v;
      }
      void sendheader(int fd,const char * path){
        char buf[128];
        std::string minfo=getinfo(path);
        //ysDebug("mime:%s",minfo.c_str());
        sprintf(buf,"Content-Type: %s\r\n",minfo.c_str());
        send(fd, buf, strlen(buf), 0);
      }
    }mimer;
  }
}
#endif