#ifndef ys_ecc
#define ys_ecc
#include <stdlib.h>
#include <math.h>
#include <climits>
#define INT64_MAX  9223372036854775807
#define INT64_MIN -9223372036854775808
class ECC{
  public:
  class E {// 表示椭圆曲线方程
    public:
    int64_t p;//模p的椭圆群
    int64_t a;
    int64_t b;
    E(){}
    E(int64_t p, int64_t a, int64_t b) {
      this->p = p;
      this->a = a;
      this->b = b;
    }
  }e;
  class Pare{
    public:
    int64_t x,y;
    E  * e;
    Pare(E * e){
      x=0;
      y=0;
      this->e=e;
    }
    Pare(const Pare & p){
      x=p.x;
      y=p.y;
      e=p.e;
    }
    Pare & operator=(const Pare & p){
      x=p.x;
      y=p.y;
      e=p.e;
      return *this;
    }
    int64_t mod(int64_t a, int64_t b) {
      a = a%b;
      while(a<0) {
        a += b;
      }
      return a;
    }
    int64_t moddivision(int64_t a, int64_t b, int64_t c) {
      a = mod(a,b);
      while(a%c != 0) {
        a += b;
      }
      a = a/c;
      return a;
    }
    Pare add(const Pare & pare){
      if(this->x == INT64_MAX) {//为无穷大时O+P=P
        return pare;
      }
      Pare res (e);
      if(this->y==pare.y && this->x==pare.x) {//相等时
        int64_t d = moddivision(3*this->x*this->x + e->a,e->p,2*this->y);
        res.x = d*d - 2*this->x;
        res.x = mod(res.x, e->p);
        res.y = d*(this->x - res.x) - this->y;
        res.y = mod(res.y, e->p);
      }
      else if(pare.x - this->x != 0) {
        int64_t d = moddivision(pare.y - this->y,e->p,pare.x - this->x);
        res.x = d*d - this->x - pare.x;
        res.x = mod(res.x, e->p);
        res.y = d*(this->x - res.x) - this->y;
        res.y = mod(res.y, e->p);
      }else {//P Q互逆,返回无穷大
        res.x = INT64_MAX;
        res.y = INT64_MAX;
      }
      return res;
    }
    Pare operator+(const Pare & p){
      return this->add(p);
    }
    Pare less(const Pare & p) {
      Pare b(p);
      b.y *= -1;
      return add(b);
    }
    Pare operator-(const Pare & p){
      return this->less(p);
    }
    Pare multiply(int64_t num) {
      Pare p (*this);
      for(int64_t i=1; i<num; i++) {
        p = p.add(*this);
      }
      return p;
    }
    Pare operator*(int64_t num){
      return this->multiply(num);
    }
  };
  class Message {//传送消息的最小单元
    public:
    Pare c1;
    Pare c2;
    Message(Pare ppa, Pare ppb):c1(ppa),c2(ppb){}
  };
  
  Pare pare;//椭圆上的已知点
  int64_t privatekey;//7位速度变慢 私钥--随机
  Pare publickey;//公钥
  static int64_t random(int64_t max){
  
  }
  static int64_t prime(){
  
  }
  ECC():e(prime(),random(1024),random(1024)),pare(&e),publickey(&e){
    privatekey = random(1024);//7位速度变慢 私钥--随机
    pare.x=random(100000000);
    pare.y=random(100000000);
    publickey = pare.multiply(privatekey);//new Pare();
  }
  ECC(const E & el,int64_t kx,int64_t ky,int64_t px,int64_t py):e(el),pare(&e),publickey(&e){
    pare.x=px;
    pare.y=py;
    publickey.x=kx;
    publickey.y=ky;
  }
  Message encryption(Pare & m) {
    int64_t r    = random(1024);//随机数
    Pare c1   = m+(publickey*r);
    Pare c2   = pare*r;
    return Message(c1,c2);
  }
  Pare decryption(Message &m) {
    return m.c1-(m.c2*privatekey);
  }
  
};
#endif