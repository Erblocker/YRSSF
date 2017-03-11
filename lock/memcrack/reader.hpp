#include <iostream>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ptrace.h>
#ifndef ysmrd
#define ysmrd
class mreader{
  int fd;
  public:
  char path[PATH_MAX];
  mreader(int pid){
    sprintf(path,"/proc/%d/mem",pid);
    fd=open(path,O_RDWR);
  }
  ~mreader(){
    close(fd);
  }
  off_t seekmem(off_t offset, int fromwhere){
    return  lseek(fd,offset,fromwhere);
  }
  int readmem(void *buf, size_t count, off_t offset){
    return pread(fd,buf,count,offset);
  }
  int writemem(void *buf, size_t count, off_t offset){
    return pwrite(fd,buf,count,offset);
  }
};
#endif