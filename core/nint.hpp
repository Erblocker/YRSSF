#ifndef yrssf_core_nint
#define yrssf_core_nint
#include "func.hpp"
namespace yrssf{
class nint32{
  public:
  int32_t value;
  nint32(){
    value=0;
  }
  nint32(int32_t v){
    value=htonl(v);
  }
  nint32 & operator=(int32_t v){
    value=htonl(v);
    return *this;
  }
  nint32(const nint32 & v){
    value=v.value;
  }
  nint32 & operator=(const nint32 & v){
    value=v.value;
    return *this;
  }
  int32_t val(){
    return ntohl(value);
  }
  int32_t operator()(){
    return val();
  }
};
class nint64{
  private:
  uint32_t a,b;
  public:
  uint64_t get(){
    return ntohl(a) * 0x100000000 + ntohl(b);
  }
  void set(uint64_t value){
    uint32_t s,z;
    s=value     / 0x100000000;
    z=value - s * 0x100000000;
    a=htonl(s);
    b=htonl(z);
  }
  nint64(){
    set(0);
  }
  nint64(const nint64 & v){
    a=v.a;
    b=v.b;
  }
  nint64 & operator=(const nint64 & v){
    a=v.a;
    b=v.b;
    return *this;
  }
  nint64(int64_t v){
    set(v);
  }
  nint64 & operator=(int64_t v){
    set(v);
    return *this;
  }
  int64_t operator()(){
    return get();
  }
};
}
#endif