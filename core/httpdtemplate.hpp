#ifndef yrssf_httpd_template
#define yrssf_httpd_template
#include <sys/file.h>
#include <unistd.h>
#include <map>
#include <string>
#include <string.h>
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
        while((len=read(fd,&buf,sizeof(buf)))>0){
          if(buf=='\0')return 1;
          if(buf=='<'){
            if(pread(fd,bufdata,32,0)>0){
              bufdata[32]='\0';
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
  }
}
#endif