#include <stdio.h>
#include <iostream>
#include "ecc.hpp"
using namespace std;
int main(){
  ECC ecc;
  cout<<"ecc init finished"<<endl;
  ECC::Message msg(ECC::Pare(&(ecc.e)),ECC::Pare(&(ecc.e)));
  cout<<"msg init finished"<<endl;
  ECC::Pare    m(&(ecc.e));
  ECC::Pare    o(&(ecc.e));
  cout<<"pare init finished"<<endl;
  m.x=200;
  m.y=99;
  msg=ecc.encryption(m);
  cout<<"encode"<<endl;
  o=ecc.decryption(msg);
  cout<<"decode"<<endl;
  cout<<ecc.e.p<<endl;
  cout<<ecc.e.a<<endl;
  cout<<ecc.e.b<<endl;
  cout<<(msg.c1.x)<<endl;
  cout<<(msg.c1.y)<<endl;
  cout<<(msg.c2.x)<<endl;
  cout<<(msg.c2.y)<<endl;
  cout<<o.x<<endl;
  cout<<o.y<<endl;
  cout<<"\n"<<endl;
  
  int64_t data=123456;
  int64_t sign[2];
  
  cout<<"sign test"<<endl;
  ecc.sign(data,sign);
  if(ecc.check(data,sign))
    cout<<"success"<<endl;
  else
    cout<<"fail"<<endl;
  sign[0]=23377;
  sign[1]=45566;
  if(ecc.check(data,sign))
    cout<<"success"<<endl;
  else
    cout<<"fail"<<endl;
  
  return 0;
}