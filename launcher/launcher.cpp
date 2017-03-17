#include "common.hpp"
int main(){
  init_daemon();
  bool lockmode=0;
  if(process_num("launcher")>2){
    exit(1);
  }
  if(access("lock",F_OK)==0){
    if(!(process_num("lock")>1))
      system("su -c \"./lock\" &");
    lockmode=1;
  }
  while(1){
    if(!(process_num("daemon")>1)){
      system("./daemon &");
    }
    if(!(process_num("YRSSF")>1)){
      system("./YRSSF &");
    }
    if(lockmode)
    if(!(process_num("lock")>1)){
      system("./lock &");
    }
    sleep(1);
  }
  return 0;
}