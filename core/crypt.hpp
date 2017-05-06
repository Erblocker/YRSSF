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
  aesblock & operator=(aesblock & f){
    for(int i=0;i<16;i++) data[i]=f.data[i];
    return *this;
  }
  void tobase64(char * out)const{
    base64::encode(data,16,(unsigned char*)out);
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
}
}
#endif