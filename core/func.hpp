#ifndef ys_core_func
#define ys_core_func
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <arpa/inet.h>
namespace yrssf{
void   int2str(const int32_t  * i,char * c){
  sprintf(c, "%8x ", *i);
  c[8]='\0';
}
void   str2int(const char * c,int32_t  * i){
  sscanf(c,  "%8x",  i); 
}
int randnum(){
  static int st=time(NULL);
  int r;
  st++;
  srand(st);
  r=rand();
  if(!r)r++;
  return r;
}
char * randstr(){
  static char result[17];
  result[16]='\0';
  char arr[]="xcvbnm,.asdfghjkl;'qwertyuiop1234567890~`/[]{}";
  for(int i=0;i<16;i++){
    result[i]=arr[randnum() % sizeof(arr)];
  }
  return result;
}
char * nowtime(){
  static char timestr[9];
  timestr[8]='\0';
  time_t t;  
  t = time(NULL);  
  struct tm *lt;  
  int ii = time(&t);
  int2str(&ii,timestr);
  return timestr;
}
int nowtime_s(){
  time_t t;  
  t = time(NULL);  
  struct tm *lt;  
  return time(&t);
}
int iptoint( const char *ip ){
  return ntohl( inet_addr( ip ) );
}
void inttoip(in_addr ip_num, char *ip ){
  int32_t inm=ip_num.s_addr;
  uint32_t s1 = ((u_char*)inm)[0];
  uint32_t s2 = ((u_char*)inm)[1];
  uint32_t s3 = ((u_char*)inm)[2];
  uint32_t s4 = ((u_char*)inm)[3];
  sprintf(ip,"%d.%d.%d.%d",s1,s2,s3,s4);
}
void wristr(const char * in,char * out){
    for(int i=0;i<16;i++){
      if(in[i]=='\0'){
        out[i]='\0';
        return;
      }
      out[i]=in[i];
    }
}
}
#endif
