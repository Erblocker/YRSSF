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
  msg.c1.x=11;
  msg.c1.y=11;
  msg.c2.x=11;
  msg.c2.y=11;
  m=ecc.decryption(msg);
  msg=ecc.encryption(m);
  cout<<(msg.c1.x)<<endl;
  cout<<(msg.c1.y)<<endl;
  cout<<(msg.c2.x)<<endl;
  cout<<(msg.c2.y)<<endl;
  cout<<m.x<<endl;
  cout<<m.y<<endl;
  cout<<"\n"<<endl;
  return 0;
}