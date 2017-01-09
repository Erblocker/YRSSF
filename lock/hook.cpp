#include <signal.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/prctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <pwd.h>
#include "config.h"
#define read_lock(fd, offset, whence, len) lock_reg((fd), F_SETLK, F_RDLCK, (offset), (whence), (len))
#define readw_lock(fd, offset, whence, len) lock_reg((fd), F_SETLKW, F_RDLCK, (offset), (whence), (len))
#define write_lock(fd, offset, whence, len) lock_reg((fd), F_SETLK, F_WRLCK, (offset), (whence), (len))
#define writew_lock(fd, offset, whence, len) lock_reg((fd), F_SETLKW, F_WRLCK, (offset), (whence), (len))
#define un_lock(fd, offset, whence, len) lock_reg((fd), F_SETLK, F_UNLCK, (offset), (whence), (len))
void   int2str(int32_t  * i,char * c){
  sprintf(c, "%8x ", *i);
  c[8]='\0';
}
void   str2int(char * c,int32_t  * i){
  sscanf(c,  "%8x",  i); 
}
void init_daemon(){
  int pid;
  int i; 
  if(pid=fork()) 
    exit(0);//是父进程，结束父进程 
  else
    if(pid< 0) 
      exit(1);//fork失败，退出 
  //是第一子进程，后台继续执行 
  setsid();//第一子进程成为新的会话组长和进程组长 
  //并与控制终端分离 
  if(pid=fork()) 
    exit(0);//是第一子进程，结束第一子进程 
  else
    if(pid< 0) 
      exit(1);//fork失败，退出 
  //是第二子进程，继续 
  //第二子进程不再是会话组长 
  for(i=0;i< NOFILE;++i)//关闭打开的文件描述符 
    close(i); 
  chdir("/tmp");//改变工作目录到/tmp 
  umask(0);//重设文件创建掩模 
  return; 
}
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
bool isindir(const char * par,const char * c){
  int i=0;
  while(1){
    if(par[i]==c[i]){
      if(par[i]=='\0'){
        return 1;
      }else{
        i++;
        continue;
      }
    }
    if(par[i]=='\0')return 1;
    return 0;
  }
}
int lock_reg(int fd, int cmd, int type, off_t offset, int whence, off_t len){
  struct flock lock;
  lock.l_type = type; //F_RDLCK, F_WRLCK, F_UNLCK
  lock.l_start = offset; //byte offset, relative to l_whence
  lock.l_whence = whence; //SEEK_SET, SEEK_CUR, SEEK_END
  lock.l_len = len; //#bytes (0 means to EOF)
  return (fcntl(fd, cmd, &lock));
}
int already_running(const char * path){
  int fd;
  char buf[32];
  fd = open(path,O_RDWR|O_CREAT,0777);
  if (fd < 0) {
    return 1;
  }
  //加建议性写锁
  if (write_lock(fd,  0, SEEK_SET, 0) < 0) {
    close(fd);
    return 1;
  }
  ftruncate(fd, 0); //文件清零，因为只写一次pid号
  snprintf(buf, 32, "%ld", (long)getpid());
  write(fd, buf, strlen(buf)+1);
  return 0;
}
void allowed(){}
void myprocess_loop(){
  while(1){
    sleep(2);
    
  }
}
void myprocess(){
  int pid;
  int i;
  int fd;
  char buf[200];
  if(fork()==0){
    init_daemon();
    prctl(PR_SET_NAME,"ydaemon",0,0,0);
    if(already_running("/tmp/yrssflocker")==0){
      myprocess_loop();
    }
    exit(0);
  }
}
class Init{
  public:
  Init(){
    char path[PATH_MAX];
    char processname[1024];
    const char ** ac;
    get_executable_path(path, processname, sizeof(path));
    myprocess();
    struct passwd *pwd;
    pwd = getpwuid(getuid());
    ac=allowuser;
    while(*ac){
      if(strcmp(*ac,pwd->pw_name)==0)goto allowed;
      ac++;
    }
    ac=allowpath;
    while(*ac){
      if(isindir(*ac,path))goto allowed;
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