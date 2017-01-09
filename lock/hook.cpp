#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
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
class Init{
  public:
  Init(){
    char path[PATH_MAX];
    char processname[1024];
    get_executable_path(path, processname, sizeof(path));
    printf("directory:%s\nprocessname:%s\n",path,processname);
  }
}init;
//extern "C"{}