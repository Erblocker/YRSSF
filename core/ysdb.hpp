#ifndef yrssf_core_ysdb
#define yrssf_core_ysdb
#include "nint.hpp"
#include "func.hpp"
#include "leveldb/db.h"
#include "leveldb/write_batch.h"
#include <list>
#include <map>
#include <set>
#include "classes.hpp"
#include "rwmutex.hpp"
#include "crypt.hpp"
#include "key.hpp"
namespace yrssf{
const char defaultUserInfo[]="1234567890abcdef1234567890abcdef1234567890abcdef";
std::string userpre="system_user_";
std::string srcpre="system_src_";
class YsDB{
  public:
  std::map<int32_t,location> userIP;
  leveldb::DB *              ldata;
  std::set<location>         livelist;
  leveldb::DB *              unique;
  RWMutex                    locker;
  RWMutex                    livelocker;
  RWMutex                    livelistlocker;
  YsDB(){
    leveldb::Options options;
    options.create_if_missing = true;
    assert(leveldb::DB::Open(options, "data/session", &unique).ok());
    assert(leveldb::DB::Open(options, "data/ldata", &ldata).ok());
  }
  ~YsDB(){
    delete unique;
    delete ldata;
  }
  void liveAdd(const location & address){
    livelocker.Wlock();
    livelist.insert(address);
    livelocker.unlock();
  }
  void liveRemove(const location & address){
    livelocker.Wlock();
    auto it=livelist.find(address);
    if(it!=livelist.end())
      livelist.erase(address);
    livelocker.unlock();
  }
  void liveClean(){
    livelocker.Wlock();
    livelist.clear();
    livelocker.unlock();
  }
  void live(void(*callback)(const location &,void*),void * arg){
    livelocker.Rlock();
    
    for(auto it=livelist.begin();it!=livelist.end();it++){
      callback(*it,arg);
    }
    
    livelocker.unlock();
  }
  bool getkey(int32_t uid,aesblock * key){
    char name[9];
    std::string v;
    int2str(&uid,name);
    if(!ldata->Get(leveldb::ReadOptions(),userpre+std::string(name)+"_key",&v).ok())return 0;
    if(v.length()<16)return 0;
    key->getbase64(v.c_str());
    return 1;
  }
  void setkey(int32_t uid,aesblock * key){
    char name[9];
    int2str(&uid,name);
    char v[32];
    key->tobase64(v);
    ldata->Put(leveldb::WriteOptions(),userpre+std::string(name)+"_key",v);
  }
  bool logunique(int32_t uid,int32_t lid){
    if(lid==0)return 1;
    char keyname[256];
    char timestr[64];
    std::string v;
    sprintf(keyname,"%d:%d",uid,lid);
    if(unique->Get(leveldb::ReadOptions(),keyname,&v).ok())
      if(!v.empty())return 0;
    sprintf(timestr,"%d",nowtime_s());
    unique->Put(leveldb::WriteOptions(),keyname,timestr);
    return 1;
  }
  bool uniqueExist(int32_t uid,int32_t lid){
    if(lid==0)return 0;
    char keyname[256];
    std::string v;
    sprintf(keyname,"%d:%d",uid,lid);
    if(unique->Get(leveldb::ReadOptions(),keyname,&v).ok())
      if(!v.empty())return 1;
    return 0;
  }
  bool userExist(int32_t userid){
    std::string v;
    char userids[9];
    int2str(&userid,userids);
    if(ldata->Get(leveldb::ReadOptions(),userpre+userids,&v).ok())
      if(!v.empty())return 1;
    return 0;
  }
  bool login(int32_t userid,const char * pwd,std::string * v){
    char name[9];
    const char * tp;
    int2str(&userid,name);
    if(!ldata->Get(leveldb::ReadOptions(),userpre+std::string(name)+"_tmp",v).ok()) goto loginTmpFail;
    if(v->empty())goto loginTmpFail;
    tp=v->c_str();
    for(int i=0;i<16;i++){
      if(pwd[i]==tp[i] && pwd[i]=='\0' && i>6)
        break;
      if(pwd[i]=='\0') goto loginTmpFail;
      if( tp[i]=='\0') goto loginTmpFail;
      if(pwd[i]!=tp[i])goto loginTmpFail;
    }
    if(!ldata->Get(leveldb::ReadOptions(),userpre+name,v).ok()) return 0;
    goto loginsucceed;
    loginTmpFail:
    if(!ldata->Get(leveldb::ReadOptions(),userpre+name,v).ok()) return 0;
    if(v->empty())return 0;
    tp=v->c_str();
    for(int i=0;i<16;i++){
      if(pwd[i]==tp[i] && pwd[i]=='\0' && i>6)
        break;
      if(pwd[i]=='\0') return 0;
      if( tp[i]=='\0') return 0;
      if(pwd[i]!=tp[i])return 0;
    }
    loginsucceed:
    ldata->Put(leveldb::WriteOptions(),userpre+std::string(name)+"_time",nowtime());
    return 1;
  }
  bool setTmpPwd(int32_t userid,char * pwd){
    int i;
    for(i=0;i<16;i++)if(pwd[i]=='\0')return 0;
    char v[17];
    char name[9];
    int2str(&userid,name);
    for(i=0;i<16;i++){
      v[i]=pwd[i];
    }
    v[16]='\0';
    ldata->Put(leveldb::WriteOptions(),userpre+std::string(name)+"_tmp",v);
    return 1;
  }
  template<typename T1,typename T2>
  void writestr(T1 a,T2 b,int l,int begin){
    int i;
    for(i=0;i<l;i++){
      a[i+begin]=b[i];
    }
  }
  template<typename T>
  bool strlencheck(T in,int l){
    int i;
    for(i=0;i<l;i++)if(in[i]=='\0')return 0;
    return 1;
  }
  bool changePwd(int32_t userid,const char * pwd){
    int i;
    for(i=0;i<16;i++)if(pwd[i]=='\0')return 0;
    std::string v;
    char name[9];
    int2str(&userid,name);
    if(!ldata->Get(leveldb::ReadOptions(),userpre+name,&v).ok())return 0;
    if(v.length()<sizeof(defaultUserInfo)) return 0;
    for(i=0;i<16;i++)v[i]=pwd[i];
    ldata->Put(leveldb::WriteOptions(),userpre+name,v);
    return 1;
  }
  bool changeAcl(int32_t userid,const char * acl){
    int i;
    for(i=0;i<16;i++)if(acl[i]=='\0')return 0;
    std::string v;
    char name[9];
    int2str(&userid,name);
    if(!ldata->Get(leveldb::ReadOptions(),userpre+name,&v).ok())return 0;
    if(v.length()<sizeof(defaultUserInfo)) return 0;
    for(i=0;i<16;i++)v[i+16]=acl[i];
    ldata->Put(leveldb::WriteOptions(),userpre+name,v);
    return 1;
  }
  bool newUser(int32_t userid){
    char name[9];
    int2str(&userid,name);
    ldata->Put(leveldb::WriteOptions(),userpre+name,defaultUserInfo);
  }
  bool delUser(int32_t userid){
    char name[9];
    int2str(&userid,name);
    ldata->Delete(leveldb::WriteOptions(),userpre+name);
  }
  bool getSrcName(char * out,const char * name){
    char str[9];
    int i;
    for(i=0;i<16;i++)if(name[i]=='\0')return 0;
    for(i=0;i<8;i++){
      out[i]=name[i];
    }
    out[8]='\0';
    return 1;
  }
  bool getSrcName(char * out,const char * name,int32_t userid){
    char str[17];
    int2str(&userid,str);
    int i;
    for(i=0;i<16;i++)if(name[i]=='\0')return 0;
    for(i=0;i<8;i++){
      out[i+8]=name[i];
    }
    out[16]='\0';
    return 1;
  }
  bool writeSrc(const char * sname,const char * src){
    char name[9];
    if(!getSrcName(name,sname))return 0;
    return ldata->Put(leveldb::WriteOptions(),srcpre+name,src).ok();
  }
  bool writeSrc(const char * sname,int32_t userid,const char * src){
    char name1[9];
    std::string v;
    if(!getSrcName(name1,sname))return 0;
    if(!ldata->Get(leveldb::ReadOptions(),srcpre+name1,&v).ok())return 0;
    char name[17];
    if(!getSrcName(name,sname,userid))return 0;
    return ldata->Put(leveldb::WriteOptions(),srcpre+name,src).ok();
  }
  bool readSrc(const char * sname,std::string * src){
    char name[9];
    if(!getSrcName(name,sname))return 0;
    return ldata->Get(leveldb::ReadOptions(),srcpre+name,src).ok();
  }
  bool readSrc(const char * sname,int32_t userid,std::string * src){
    char name1[9];
    std::string v;
    if(!getSrcName(name1,sname))return 0;
    if(!ldata->Get(leveldb::ReadOptions(),srcpre+name1,&v).ok())return 0;
    char name[17];
    if(!getSrcName(name,sname,userid))return 0;
    return ldata->Get(leveldb::ReadOptions(),srcpre+name,src).ok();
  }
  bool delSrc(const char * sname){
    char name[9];
    if(!getSrcName(name,sname))return 0;
    return ldata->Delete(leveldb::WriteOptions(),srcpre+name).ok();
  }
  bool delSrc(const char * sname,int32_t userid){
    char name1[9];
    std::string v;
    if(!getSrcName(name1,sname))return 0;
    if(!ldata->Get(leveldb::ReadOptions(),srcpre+name1,&v).ok())return 0;
    char name[17];
    if(!getSrcName(name,sname,userid))return 0;
    return ldata->Delete(leveldb::WriteOptions(),srcpre+name).ok();
  }
  bool existSrc(const char * sname){
    std::string str;
    if(!readSrc(sname,&str)) return 0;
    if(str.empty()) return 0;
    return 1;
  }
  bool existSrc(const char * sname,int32_t userid){
    std::string str;
    if(!readSrc(sname,userid,&str)) return 0;
    if(str.empty()) return 0;
    return 1;
  }
  void logIP(int32_t userid,in_addr ip,short port){
    locker.Wlock();
    location p;
    p.ip=ip;
    p.port=port;
    userIP[userid]=p;
    locker.unlock();
  }
  bool getUserIP(int32_t userid,in_addr *ip,short *port){
    locker.Rlock();
    auto pt=userIP.find(userid);
    if(pt==userIP.end()){
      *port=0;
      ip->s_addr=0;
      locker.unlock();
      return 0;
    }
    location &p=pt->second;
    *ip=p.ip;
    *port=p.port;
    locker.unlock();
    return 1;
  }
}ysDB;

struct netReg{
  char          crypt;
  struct{
    nint32 uid;
    nint32 tm;
    char   pwd[16];
  }data;
  char          endchunk[32];
  nint32        slen;
  unsigned char sign[1024];
  unsigned char key[ECDH_SIZE];
  void encrypt(){
    if(crypt=='t')return;
    aesblock buf;
    char kb[ECDH_SIZE];
    DH.computekey(key,kb);
    crypt='t';
    auto here=(aesblock*)&data;
    for(int i=0;i<3;i++){
      AES128_ECB_encrypt(
        (const uint8_t *)here->data,
        (const uint8_t *)kb,
        (uint8_t *)buf.data
      );
      memcpy(here,&buf,sizeof(aesblock));
      here++;
    }
  }
  void decrypt(){
    if(crypt!='t')return;
    aesblock buf;
    char kb[ECDH_SIZE];
    DH.computekey(key,kb);
    crypt='f';
    auto here=(aesblock*)&data;
    for(int i=0;i<3;i++){
      AES128_ECB_decrypt(
        (const uint8_t *)here->data,
        (const uint8_t *)kb,
        (uint8_t *)buf.data
      );
      memcpy(here,&buf,sizeof(aesblock));
      here++;
    }
  }
  bool run(){
    if(crypt=='t')this->encrypt();
    if(!check())return 0;
    if(!ysDB.userExist(data.uid()))
      ysDB.newUser(data.uid());
    ysDB.setTmpPwd(data.uid(),data.pwd);
  }
  void dosign(){
    unsigned int slenb;
    signer.sign(
      (unsigned char *)&data,
      sizeof(data),
      (unsigned char *)sign,
      &slenb
    );
    slen=slenb;
  }
  void init(int32_t userid,const char * pwd){
    int i;
    for(i=0;i<16;i++){
      data.pwd[i]=pwd[i];
    }
    data.uid=userid;
    data.tm=nowtime_s();
    crypt='f';
    dosign();
  }
  private:
  bool check(){
    if(slen()>1024) return 0;
    if(abs(data.tm()-nowtime_s())>3600)
      return 0;
    return signer.check(
      (unsigned char *)&data,
      sizeof(data),
      sign,
      slen()
    );
  }
}myUniKey;
}
#endif