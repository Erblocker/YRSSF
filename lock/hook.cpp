#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include "config.h"
size_t get_executable_path( char* processdir,char* processname, size_t len){
        char* path_end;
        if(readlink("/proc/self/exe", processdir,len) <=0)
                return -1;
        path_end = strrchr(processdir,  '/');
        if(path_end == NULL)
                return -1;
        ++path_end;
        strcpy(processname, path_end);
        *path_end = '\0';
        return (size_t)(path_end - processdir);
}
void allowed(){}
class Init{
  public:
  Init(){
    char path[PATH_MAX];
    char processname[1024];
    const char ** ac;
    get_executable_path(path, processname, sizeof(path));
    ac=allowpath;
    while(*ac){
      if(strcmp(*ac,path)==0)goto allowed;
      ac++;
    }
    ac=allowprocess;
    while(*ac){
      if(strcmp(*ac,processname)==0)goto allowed;
      ac++;
    }
    exit(1);
    allowed:
    allowed();
  }
}init;
//extern "C"{}