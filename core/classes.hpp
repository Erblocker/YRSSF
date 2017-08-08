#ifndef yrssf_core_classes
#define yrssf_core_classes
#include "global.hpp"
#include "nint.hpp"
#include "func.hpp"
extern "C" {
#include "lua/src/lua.h"
#include "lua/src/lualib.h"
#include "lua/src/lauxlib.h"
#include "lua/src/luaconf.h"
}
#define _SUCCEED         0x30
#define _FAIL            0x31
#define _NEWSRC          0x32
#define _GETSRC          0x33
#define _DELSRC          0x34
#define _SETSRC_APPEND   0x35
#define _SETACL          0x36
#define _SETPWD          0x37
#define _LOGIN           0x38
#define _NEWUSER         0x39
#define _PLUS            0x3A
#define _GETSRCNAME      0x3B
#define _DELUSER         0x3C
#define _CONNECTUSER     0x60
#define _P2PCONNECT      0x61
#define _P2PCONNECT_C    0x62
#define _LOGIP           0x63
#define _GOLAST          0x64
#define _SHELL           0xBB
#define _GETSERVERINFO   0xAA
#define _CONNECT         0xAB
#define _UPDATEKEY       0xAC
#define _LIVESRC         0xAD
#define _LIVE            0xAE
#define _REGISTER        0xAF
#define _CERT            0xB0
#define _GET_PUBLIC_KEY  0xB1
#define _LIVE_B          0xB2
#define _LIVE_E          0xB3
namespace yrssf{
class location{
  public:
  in_addr ip;
  short   port;
  location(){
    ip.s_addr = htonl(INADDR_ANY); 
    port = 0; 
  }
  location(in_addr a,short p){
    ip=a;
    port=p;
  }
  location(const location & i){
    ip=i.ip;
    port=i.port;
  }
  location & operator=(const location & i){
    ip=i.ip;
    port=i.port;
    return *this;
  }
  int64_t tolongnum()const{
    int64_t n,b1,b2;
    int64_t a=ip.s_addr;
    int64_t p=port;
    b1=(((a) & 0xffffffff)<<32);
    b2=p & 0xffffffff;
    n=b1 | b2;
    return n;
  }
  bool operator==(const location & i)const{
    return ((this->tolongnum())==(i.tolongnum()));
  }
  bool operator!=(const location & i)const{
    return !((*this)==i);
  }
  bool operator<(const location & i)const{
    return ((this->tolongnum())<(i.tolongnum()));
  }
  bool operator>(const location & i)const{
    return ((this->tolongnum())>(i.tolongnum()));
  }
};
struct netHeader{
  char          crypt;
  nint32        userid;
  int32_t       unique;
  nint32        len;
  nint32        hash;
  char          password[16];
  unsigned char mode;
  char          globalMode;
  char          function[16];
};
struct netQuery{
  netHeader header;
  char      str1[16];
  char      str2[16];
  in_addr   addr;
  nint32    num1;
  nint32    num2;
  nint32    num3;
  nint32    num4;
  char      endchunk[16];
};
struct netSource{
  netHeader header;
  nint32    size;
  char      title[16];
  char      source[SOURCE_CHUNK_SIZE];
  char      endchunk[16];
};
}
#endif