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
#define BUFSZ PIPE_BUF
int process_num(const char * pname){
  FILE* fp;
  int count;
  char buf[BUFSZ];
  char command[256];
  sprintf(command, "ps -C %s|wc -l",pname);
  //printf(command);
  if((fp = popen(command,"r")) == NULL)
    return 0;
  if( (fgets(buf,BUFSZ,fp))!= NULL ){
    count = atoi(buf);
    //printf("\n %d \n",count);
    pclose(fp);
    return count;
  }
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
  //chdir("/tmp");//改变工作目录到/tmp 
  umask(0);//重设文件创建掩模 
  return; 
}
void run(const char * command){
  FILE * fp = popen(command,"r");
  pclose(fp);
}