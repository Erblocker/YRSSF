#ifndef yrssf_core_filecache
#define yrssf_core_filecache
#include "global.hpp"
#include "cache.hpp"
#include <stdio.h>
#include <memory.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/file.h>
namespace yrssf{
  namespace filecache{
    size_t maxsize=65536;
    void cache_onfree(cache::value & self){
      free(self.data);
    }
    void addcache(const char * name,void * mb,int length){
      cache::value v(300);
      v.data=mb;
      v.length=length;
      v.onfree=cache_onfree;
      cache::locker.Wlock();
      cache::cache[name]=v;
      cache::locker.unlock();
    }
    bool getcache(const char * name,unsigned int begin,unsigned int len,void * buf,int *rl){
      char * buffer=(char*)buf;
      cache::locker.Rlock();
      auto it=cache::cache.find(name);
      if(it==cache::cache.end()){
        cache::locker.unlock();
        return 0;
      }
      auto v=it->second;
      if(v.data==NULL){
        cache::locker.unlock();
        return 0;
      }
      auto cp=(char*)v.data;
      if(v.length==0){
        cache::locker.unlock();
        return 0;
      }
      if(v.length<begin){
        cache::locker.unlock();
        return -1;
      }
      int leng=(begin+len)>v.length ? v.length-begin : len;
      *rl=leng;
      for(int i=0;i<leng;i++){
        buffer[i]=cp[i+begin];
      }
      cache::locker.unlock();
      return 1;
    }
    bool addfileintobuffer(const char * path){
      struct stat s;
      stat(path,&s);
      int fd=open(path,O_RDONLY);
      if(fd==-1)return 0;
      if(s.st_size>maxsize || s.st_size==0)
        return 0;
      void * buf=malloc(s.st_size);
      read(fd,buf,s.st_size);
      close(fd);
      return 1;
    }
    int getfile(const char *path,int sk,void * buf,size_t len){
      int fd=open(path,O_RDONLY);
      lseek(fd,sk,SEEK_SET);
      int res=read(fd,buf,len);
      close(fd);
      return res;
    }
    void removefilefrombuffer(const char * path){
      cache::locker.Wlock();
      cache::cache.find(path);
      cache::locker.unlock();
    }
    void fileappend(const char * n,char * data,unsigned int size){
      removefilefrombuffer(n);
      FILE * f=fopen(n,"a");
      fwrite(data,size,1,f);
      fclose(f);
    }
    int readfile(const char * path,unsigned int begin,unsigned int len,void * buf){
      int rl;
      int res=getcache(path,begin,len,buf,&rl);
      if(res==1){
        return rl;
      }else if(res==-1){
        return -1;
      }else
        addfileintobuffer(path);
      return(getfile(path,begin,buf,len));
    }
    int removefile(const char * path){
      removefilefrombuffer(path);
      return remove(path);
    }
  }
}
#endif