#include "common.hpp"
int main(){
  init_daemon();
  if(process_num("daemon")>2){
    exit(1);
  }
  while(1){
    if(!(process_num("launcher")>1)){
      system("./launcher &");
    }
    sleep(1);
  }
  return 0;
}