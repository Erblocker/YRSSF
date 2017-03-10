#include "common.hpp"
int main(){
  init_daemon();
  if(process_num("launcher")>2){
    exit(1);
  }
  while(1){
    if(!(process_num("daemon")>1)){
      system("./daemon &");
    }
    if(!(process_num("YRSSF")>1)){
      system("./YRSSF &");
    }
    sleep(1);
  }
  return 0;
}