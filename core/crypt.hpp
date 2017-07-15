#ifndef yrssf_core_crypt
#define yrssf_core_crypt
#include "nint.hpp"
#include "func.hpp"
extern "C"{
#include "aes/aes.h"
}
#include "base64.hpp"
#include <string.h>
//#include "ysdb.hpp"
namespace yrssf{
struct aesblock{
  uint8_t data[16];
  aesblock & operator=(const aesblock & f){
    for(int i=0;i<16;i++) data[i]=f.data[i];
    return *this;
  }
  void tobase64(char * out)const{
    base64::encode(data,16,(unsigned char*)out);
  }
  std::string tobase64()const{
    char buf[64];
    tobase64(buf);
    return std::string(buf);
  }
  void getbase64(const char * in){
    char buf[64];
    if(strlen(in)>64)return;
    base64::decode((const unsigned char*)in,(unsigned char*)buf);
    for(int i=0;i<16;i++){
      data[i]=buf[i];
    }
  }
};
template<typename T>
void crypt_encode(T data,aesblock * key){
  if(data->header.crypt=='t')return;
  register aesblock * here =(aesblock*)&(data->header.unique);
  register aesblock * end  =(aesblock*)&(data->endchunk[0]);
  aesblock buf;
  data->header.crypt='t';
  
  auto begin_d=(char*)&(data->header.password);
  auto end_d  =(char*)&(data->endchunk[0]);
  int len=end_d-begin_d;
  data->header.len=len;
  data->header.hash=RSHash(begin_d,len);
  
  while(here<end){
    AES128_ECB_encrypt(here->data, key->data, buf.data);
    memcpy(here,&buf,sizeof(aesblock));
    here++;
  }
}
template<typename T>
void crypt_decode(T * data,aesblock * key){
  if(data->header.crypt!='t')return;
  register aesblock * here =(aesblock*)&(data->header.unique);
  register aesblock * end  =(aesblock*)&(data->endchunk[0]);
  aesblock buf;
  data->header.crypt='f';
  while(here<end){
    AES128_ECB_decrypt(here->data, key->data, buf.data);
    memcpy(here,&buf,sizeof(aesblock));
    here++;
  }
  
  int hash_d=data->header.hash();
  auto begin_d=(char*)&(data->header.password);
  auto end_d  =(char*)&(data->endchunk[0]);
  int len=data->header.len();
  if(((end_d-begin_d)<len) || len<0){ //恶意伪造长度
    bzero(data,sizeof(*data));
    return;
  }
  int hash=RSHash(begin_d,len);
  
  if(hash_d!=hash){
    bzero(data,sizeof(*data));
  }
}
}
#endif