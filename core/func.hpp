#ifndef ys_core_func
#define ys_core_func
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <arpa/inet.h>
#include "global.hpp"
#include "cppjieba/limonp/Md5.hpp"
#include <string.h>
namespace yrssf{
///////////////////////////////////////////
inline int lua_isptr(lua_State * L,int p){
  return (lua_isuserdata(L,p));
}
inline void * lua_toptr(lua_State * L,int p){
  if(lua_isuserdata(L,p)){
    auto pt=(void **)lua_touserdata(L,p);
    return *pt;
  }else{
    return NULL;
  }
}
inline void lua_pushptr(lua_State * L,void * ptr){
  auto pt=(void **)lua_newuserdata(L,sizeof(void*));
  *pt=ptr;
}
namespace math{
  double invsqrt(double x){
    double xhalf=0.5f*x;
    int i=*(int*)&x;
    i=0x5f3759df-(i>>1);
    double y=*(double*)&i;
    return (y*(1.5f-xhalf*x*x));
  }
  double sqrt(double x){
    return (1.0f/invsqrt(x));
  }
}

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
  char arr[]="xcvbnmasdfghjklqwertyuiop"
             "1234567890ZXCVBNMASDFGHJK"
             "LQWERTYUIOP";
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
unsigned int RSHash(const char* str, unsigned int len){
   unsigned int b    = 378551;
   unsigned int a    = 63689;
   unsigned int hash = 0;
   unsigned int i    = 0;
   for(i = 0; i < len; str++, i++){
      hash = hash * a + (*str);
      a    = a * b;
   }
   return hash;
}
int lua_RSHash(lua_State * L){
  if(!lua_isstring(L,1))return 0;
  const char * str=lua_tostring(L,1);
  lua_pushinteger(L,RSHash(str,strlen(str)));
  return 1;
}
unsigned int JSHash(const char* str, unsigned int len){
   unsigned int hash = 1315423911;
   unsigned int i    = 0;
   for(i = 0; i < len; str++, i++){
      hash ^= ((hash << 5) + (*str) + (hash >> 2));
   }
   return hash;
}
int lua_JSHash(lua_State * L){
  if(!lua_isstring(L,1))return 0;
  const char * str=lua_tostring(L,1);
  lua_pushinteger(L,JSHash(str,strlen(str)));
  return 1;
}
unsigned int PJWHash(const char* str, unsigned int len){
   const unsigned int BitsInUnsignedInt = (unsigned int)(sizeof(unsigned int) * 8);
   const unsigned int ThreeQuarters     = (unsigned int)((BitsInUnsignedInt  * 3) / 4);
   const unsigned int OneEighth         = (unsigned int)(BitsInUnsignedInt / 8);
   const unsigned int HighBits          = (unsigned int)(0xFFFFFFFF) << (BitsInUnsignedInt - OneEighth);
   unsigned int hash              = 0;
   unsigned int test              = 0;
   unsigned int i                 = 0;
   for(i = 0; i < len; str++, i++){
      hash = (hash << OneEighth) + (*str);
      if((test = hash & HighBits)  != 0){
         hash = (( hash ^ (test >> ThreeQuarters)) & (~HighBits));
      }
   }
   return hash;
}
int lua_PJWHash(lua_State * L){
  if(!lua_isstring(L,1))return 0;
  const char * str=lua_tostring(L,1);
  lua_pushinteger(L,PJWHash(str,strlen(str)));
  return 1;
}
unsigned int ELFHash(const char* str, unsigned int len){
   unsigned int hash = 0;
   unsigned int x    = 0;
   unsigned int i    = 0;
   for(i = 0; i < len; str++, i++){
      hash = (hash << 4) + (*str);
      if((x = hash & 0xF0000000L) != 0){
         hash ^= (x >> 24);
      }
      hash &= ~x;
   }
   return hash;
}
int lua_ELFHash(lua_State * L){
  if(!lua_isstring(L,1))return 0;
  const char * str=lua_tostring(L,1);
  lua_pushinteger(L,ELFHash(str,strlen(str)));
  return 1;
}
unsigned int BKDRHash(const char* str, unsigned int len){
   unsigned int seed = 131; /* 31 131 1313 13131 131313 etc.. */
   unsigned int hash = 0;
   unsigned int i    = 0;
   for(i = 0; i < len; str++, i++){
      hash = (hash * seed) + (*str);
   }
   return hash;
}
int lua_BKDRHash(lua_State * L){
  if(!lua_isstring(L,1))return 0;
  const char * str=lua_tostring(L,1);
  lua_pushinteger(L,BKDRHash(str,strlen(str)));
  return 1;
}
unsigned int SDBMHash(const char* str, unsigned int len){
   unsigned int hash = 0;
   unsigned int i    = 0;
   for(i = 0; i < len; str++, i++){
      hash = (*str) + (hash << 6) + (hash << 16) - hash;
   }
   return hash;
}
int lua_SDBMHash(lua_State * L){
  if(!lua_isstring(L,1))return 0;
  const char * str=lua_tostring(L,1);
  lua_pushinteger(L,SDBMHash(str,strlen(str)));
  return 1;
}
unsigned int DJBHash(const char* str, unsigned int len){
   unsigned int hash = 5381;
   unsigned int i    = 0;
   for(i = 0; i < len; str++, i++){
      hash = ((hash << 5) + hash) + (*str);
   }
   return hash;
}
int lua_DJBHash(lua_State * L){
  if(!lua_isstring(L,1))return 0;
  const char * str=lua_tostring(L,1);
  lua_pushinteger(L,DJBHash(str,strlen(str)));
  return 1;
}
unsigned int DEKHash(const char* str, unsigned int len){
   unsigned int hash = len;
   unsigned int i    = 0;
   for(i = 0; i < len; str++, i++){
      hash = ((hash << 5) ^ (hash >> 27)) ^ (*str);
   }
   return hash;
}
int lua_DEKHash(lua_State * L){
  if(!lua_isstring(L,1))return 0;
  const char * str=lua_tostring(L,1);
  lua_pushinteger(L,DEKHash(str,strlen(str)));
  return 1;
}
unsigned int BPHash(const char* str, unsigned int len){
   unsigned int hash = 0;
   unsigned int i    = 0;
   for(i = 0; i < len; str++, i++){
      hash = hash << 7 ^ (*str);
   }
   return hash;
}
int lua_BPHash(lua_State * L){
  if(!lua_isstring(L,1))return 0;
  const char * str=lua_tostring(L,1);
  lua_pushinteger(L,BPHash(str,strlen(str)));
  return 1;
}
unsigned int FNVHash(const char* str, unsigned int len)
{
   const unsigned int fnv_prime = 0x811C9DC5;
   unsigned int hash      = 0;
   unsigned int i         = 0;
   for(i = 0; i < len; str++, i++){
      hash *= fnv_prime;
      hash ^= (*str);
   }
   return hash;
}
int lua_FNVHash(lua_State * L){
  if(!lua_isstring(L,1))return 0;
  const char * str=lua_tostring(L,1);
  lua_pushinteger(L,FNVHash(str,strlen(str)));
  return 1;
}
unsigned int APHash(const char* str, unsigned int len){
   unsigned int hash = 0xAAAAAAAA;
   unsigned int i    = 0;
   for(i = 0; i < len; str++, i++){
      hash ^= ((i & 1) == 0) ? (  (hash <<  7) ^ (*str) * (hash >> 3)) :
                               (~((hash << 11) + ((*str) ^ (hash >> 5))));
   }
   return hash;
}
int lua_APHash(lua_State * L){
  if(!lua_isstring(L,1))return 0;
  const char * str=lua_tostring(L,1);
  lua_pushinteger(L,APHash(str,strlen(str)));
  return 1;
}
template<typename T> class vec3{
  public:
    T x;
    T y;
    T z;
    vec3(){
      x=0.0f;
      y=0.0f;
      z=0.0f;
    }
    vec3(T xt,T yt,T zt){
      x=xt;
      y=yt;
      z=zt;
    }
    virtual void init(T xt,T yt,T zt){
      x=xt;
      y=yt;
      z=zt;
    }
    virtual bool operator==(vec3<T> &i){
      if(x!=i.x)return false;
      if(y!=i.y)return false;
      if(z!=i.z)return false;
      return true;
    }
    virtual void operator()(T xt,T yt,T zt){
      init(xt,yt,zt);
    }
    virtual vec3<T>& operator=(vec3<T> *p){
      x=p->x;
      y=p->y;
      z=p->z;
      return *this;
    }
    virtual vec3<T>& operator=(const vec3<T> &p){
      x=p.x;
      y=p.y;
      z=p.z;
      return *this;
    }
    virtual vec3<T> operator+(vec3<T> &p){
      vec3<T> b;
      b=this;
      b.x+=p.x;
      b.y+=p.y;
      b.z+=p.z;
      return b;
    }
    virtual vec3<T> operator-(vec3<T> &p){
      vec3<T> b;
      b=this;
      b.x-=p.x;
      b.y-=p.y;
      b.z-=p.z;
      return b;
    }
    virtual vec3<T> operator*(T &p){
      return vec3<T>(p*x , p*y , p*z);
    }
    virtual vec3<T> operator/(T &p){
      return vec3<T>(x/p , y/p , z/p);
    }
    virtual vec3<T> operator*(vec3<T> &i){
      return vec3<T>(
        y * i.z - z * i.y,
        z * i.x - x * i.z,
        z * i.y - y * i.x
      );
    }
    virtual T norm(){
      return math::sqrt((x*x)+(y*y)+(z*z));
    }
    virtual T invnorm(){
      return math::invsqrt((x*x)+(y*y)+(z*z));
    }
    virtual T pro(vec3<T> *p){
      return math::sqrt(
        (x*p->x)+
        (y*p->y)+
        (z*p->z)
      );
    }
    virtual T pro(vec3<T> &p){
      return math::sqrt(
        (x*p.x)+
        (y*p.y)+
        (z*p.z)
      );
    }
    virtual void GeoHash(T length,char * str,int l){
      vec3<T> v;
      GeoHash(length,str,0,l,&v);
    }
    virtual void GeoHash(T length,char str[],int begin,int end,vec3<T> *zero){
      if(begin==end){
        str[begin]=0x00;
        return;
      }
      if(x>zero->x){
        if(y>zero->y){
          if(z>zero->z){
            str[begin]='a';
            zero->x+=length;
            zero->y+=length;
            zero->z+=length;
            GeoHash(length*0.5f,str,begin+1,end,zero);
          }else{
            str[begin]='b';
            zero->x+=length;
            zero->y+=length;
            zero->z-=length;
            GeoHash(length*0.5f,str,begin+1,end,zero);
          }
        }else{
          if(z>zero->z){
            str[begin]='c';
            zero->x+=length;
            zero->y-=length;
            zero->z+=length;
            GeoHash(length*0.5f,str,begin+1,end,zero);
          }else{
            str[begin]='d';
            zero->x+=length;
            zero->y-=length;
            zero->z-=length;
            GeoHash(length*0.5f,str,begin+1,end,zero);
          }
        }
      }else{
        if(y>zero->y){
          if(z>zero->z){
            str[begin]='e';
            zero->x-=length;
            zero->y+=length;
            zero->z+=length;
            GeoHash(length*0.5f,str,begin+1,end,zero);
          }else{
            str[begin]='f';
            zero->x-=length;
            zero->y+=length;
            zero->z-=length;
            GeoHash(length*0.5f,str,begin+1,end,zero);
          }
        }else{
          if(z>zero->z){
            str[begin]='g';
            zero->x-=length;
            zero->y-=length;
            zero->z+=length;
            GeoHash(length*0.5f,str,begin+1,end,zero);
          }else{
            str[begin]='h';
            zero->x-=length;
            zero->y-=length;
            zero->z-=length;
            GeoHash(length*0.5f,str,begin+1,end,zero);
          }
        }
      }
    }
    virtual bool GeoHashDecode(T length,const char * s){
      const char * str=s;
      T ll=length;
      x=0.0f;
      y=0.0f;
      z=0.0f;
      while(*str){
        switch(*str){
          case ('a'):
            x+=ll;
            y+=ll;
            z+=ll;
          break;
          case ('b'):
            x+=ll;
            y+=ll;
            z-=ll;
          break;
          case ('c'):
            x+=ll;
            y-=ll;
            z+=ll;
          break;
          case ('d'):
            x+=ll;
            y-=ll;
            z-=ll;
          break;
          case ('e'):
            x-=ll;
            y+=ll;
            z+=ll;
          break;
          case ('f'):
            x-=ll;
            y+=ll;
            z-=ll;
          break;
          case ('g'):
            x-=ll;
            y-=ll;
            z+=ll;
          break;
          case ('h'):
            x-=ll;
            y-=ll;
            z-=ll;
          break;
          default:
            return false;
          break;
        }
        ll=ll*0.5f;
        str++;
      }
      return true;
    }
};
int lua_geohash_encode(lua_State * L){
  if(!lua_isnumber(L,1))return 0;
  if(!lua_isnumber(L,2))return 0;
  if(!lua_isnumber(L,3))return 0;
  if(!lua_isnumber(L,4))return 0;
  vec3<double> position(
    lua_tonumber(L,1),
    lua_tonumber(L,2),
    lua_tonumber(L,3)
  );
  char data[65];
  position.GeoHash(
    lua_tonumber(L,4),
    data,64
  );
  lua_pushstring(L,data);
  return 1;
}
int lua_geohash_decode(lua_State * L){
  if(!lua_isstring(L,1))return 0;
  if(!lua_isnumber(L,2))return 0;
  vec3<double> position;
  position.GeoHashDecode(
    lua_tonumber(L,2),
    lua_tostring(L,2)
  );
  lua_pushnumber(L,position.x);
  lua_pushnumber(L,position.y);
  lua_pushnumber(L,position.z);
  return 3;
}

unsigned char hexchars[] = "0123456789ABCDEF";

int htoi(char *s){
    int value;
    int c;

    c = ((unsigned char *)s)[0];
    if (isupper(c))
        c = tolower(c);
    value = (c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10) * 16;

    c = ((unsigned char *)s)[1];
    if (isupper(c))
        c = tolower(c);
    value += c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10;

    return (value);
}


char * url_encode(char const *s, int len, int *new_length){
    register unsigned char c;
    unsigned char *to, *start;
    unsigned char const *from, *end;
    
    from = (unsigned char *)s;
    end  = (unsigned char *)s + len;
    start = to = (unsigned char *) calloc(1, 3*len+1);

    while (from < end){
        c = *from++;

        if (c == ' '){
            *to++ = '+';
        }else if ((c < '0' && c != '-' && c != '.') ||
                 (c < 'A' && c > '9') ||
                 (c > 'Z' && c < 'a' && c != '_') ||
                 (c > 'z')){
            to[0] = '%';
            to[1] = hexchars[c >> 4];
            to[2] = hexchars[c & 15];
            to += 3;
        }else{
            *to++ = c;
        }
    }
    *to = 0;
    if (new_length){
        *new_length = to - start;
    }
    return (char *) start;
}

//匹配前缀
//s1:前缀
//s2:字符串
bool prefix_match(const char * s1,const char * s2){
      const char * sp=s1;
      const char * p2=s2;
      while(*sp){
       if((*sp)!=(*p2))return 0;
       sp++;
       p2++;
      }
      return 1;
}

int url_decode(char *str, int len){
    char *dest = str;
    char *data = str;

    while (len--){
        if (*data == '+'){
            *dest = ' ';
        }
        else if (*data == '%' && len >= 2 && isxdigit((int) *(data + 1)) && isxdigit((int) *(data + 2))){
            *dest = (char) htoi(data + 1);
            data += 2;
            len -= 2;
        }else{
            *dest = *data;
        }
        data++;
        dest++;
    }
    *dest = '\0';
    return dest - str;
}

int luaopen_ysfunc(lua_State * L){
  luaL_Reg hashreg[]={
    {"RSHash",lua_RSHash},
    {"PJWHash",lua_PJWHash},
    {"JSHash",lua_JSHash},
    {"ELFHash",lua_ELFHash},
    {"BKDRHash",lua_BKDRHash},
    {"SDBMHash",lua_SDBMHash},
    {"DJBHash",lua_DJBHash},
    {"DEKHash",lua_DEKHash},
    {"BPHash",lua_BPHash},
    {"FNVHash",lua_FNVHash},
    {"APHash",lua_APHash},
    {"md5",[](lua_State * L){
        if(!lua_isstring(L,1))return 0;
        const char * str=lua_tostring(L,1);
        std::string res;
        limonp::md5String(str,res);
        lua_pushstring(L,res.c_str());
        return 1;
      }
    },
    {"GeohashEncode",lua_geohash_encode},
    {"GeohashDecode",lua_geohash_decode},
    {NULL,NULL}
  };
  lua_newtable(L);
  luaL_setfuncs(L, hashreg, 0);
  lua_setglobal(L,"Hash");
  
  return 0;
}
/////////////////////////////////////////
}
#endif
