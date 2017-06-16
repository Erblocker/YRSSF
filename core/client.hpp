#ifndef yrssf_core_client
#define yrssf_core_client
#include "server.hpp"
namespace yrssf{
class Client:public Server{
  public:
  bool liveclientrunning;
  char parpbk[ECDH_SIZE];
  Client(short p):Server(p){
    globalmode='f';
    iscrypt=0;
    script="client.lua";
    liveclientrunning=0;
  }
  template<typename T1,typename T2>
  bool query(T1 * qypk,T2 * buf){
    int      i;
    in_addr  from;
    short    port;
    
    int rdn=randnum();
    qypk->header.unique=rdn;
    
    bzero(buf ,sizeof(T2));
    if(iscrypt)crypt_encode(qypk,&key);
    for(i=0;i<10;i++){
      send(parIP,parPort,qypk,sizeof(T1));
      loop1:
      if(recv_within_time(&from,&port,buf,sizeof(T2),1,0)){
        if(from.s_addr==parIP.s_addr && port==parPort){
          crypt_decode(&buf,&key);
          
          if(rdn!=qypk->header.unique) goto loop1;
          return 1;
        }
        else
          goto loop1;
      }
    }
    return 0;
  }
  bool getPbk(){
    netSource qypk;
    netSource buf;
    int i;
    in_addr  from;
    short    port;
    bzero(&qypk,sizeof(qypk));
    qypk.header.mode=_GET_PUBLIC_KEY;
    qypk.header.userid=myuserid;
    int rdn=randnum();
    qypk.header.unique=rdn;
    wristr(mypassword,qypk.header.password);
    if(iscrypt)crypt_encode(&qypk,&key);
    for(i=0;i<10;i++){
      send(parIP,parPort,&qypk,sizeof(qypk));
      dsloop1:
      if(recv_within_time(&from,&port,&buf,sizeof(buf),1,0)){
        if(from.s_addr==parIP.s_addr && port==parPort){
          crypt_decode(&buf,&key);
          
          if(rdn!=qypk.header.unique) goto dsloop1;
          
          if(buf.header.mode==_GET_PUBLIC_KEY){
            memcpy(
              parpbk,
              buf.source,
              ECDH_SIZE
            );
            return 1;
          }else{
            return 0;
          }
        }
        else
          goto dsloop1;
      }
    }
    return 0;
  }
  bool getUniKey(){
    netQuery qypk;
    netSource buf;
    int i;
    in_addr  from;
    short    port;
    bzero(&qypk,sizeof(qypk));
    qypk.header.mode=_CERT;
    qypk.header.userid=myuserid;
    int rdn=randnum();
    qypk.header.unique=rdn;
    wristr(mypassword,qypk.header.password);
    if(iscrypt)crypt_encode(&qypk,&key);
    for(i=0;i<10;i++){
      send(parIP,parPort,&qypk,sizeof(qypk));
      dsloop1:
      if(recv_within_time(&from,&port,&buf,sizeof(buf),1,0)){
        if(from.s_addr==parIP.s_addr && port==parPort){
          crypt_decode(&buf,&key);
          
          if(rdn!=qypk.header.unique) goto dsloop1;
          
          if(buf.header.mode==_CERT){
            memcpy(
              &myUniKey,
              buf.source,
              sizeof(netReg)
            );
            myUniKey.decrypt();
            return 1;
          }else{
            return 0;
          }
        }
        else
          goto dsloop1;
      }
    }
    return 0;
  }
  bool reg(const netReg * k){
    netSource respk;
    netQuery  qypk;
    
    netSource buf;
    int i;
    in_addr  from;
    short    port;
    
    bzero(&qypk,sizeof(qypk));
    qypk.header.mode=_REGISTER;
    qypk.header.userid=myuserid;
    int rdn=randnum();
    qypk.header.unique=rdn;
    wristr(mypassword,qypk.header.password);
    
    if(!getPbk())return 0;
    
    auto tR=(netReg*)respk.source;
    memcpy(tR->key,parpbk,ECDH_SIZE);
    tR->encrypt();
    
    if(iscrypt)crypt_encode(&qypk,&key);
    for(i=0;i<10;i++){
      send(parIP,parPort,&qypk,sizeof(qypk));
      uploop2:
      if(recv_within_time(&from,&port,&buf,sizeof(buf),1,0)){
        if(from.s_addr==parIP.s_addr && port==parPort){
          crypt_decode(&buf,&key);
          
          if(rdn!=qypk.header.unique) goto uploop2;
          
          if(buf.header.mode==_SUCCEED){
            return 1;
          }else
            return 0;
          
          return 1;
        }
        else
          goto uploop2;
      }
    }
    return 0;
  }
  bool reg(const char * path){
    netReg buf;
    int fd=open(path,O_RDONLY);
    if(fd==-1)return 0;
    read(fd,&buf,sizeof(netReg));
    close(fd);
    return reg(&buf);
  }
  void live(const char * data,uint32_t size){
    netSource qypk;
    uint32_t  len;
    
    qypk.header.mode=_LIVESRC;
    qypk.header.userid=myuserid;
    
    int rdn=randnum();
    qypk.header.unique=rdn;
    
    len = size>SOURCE_CHUNK_SIZE ? SOURCE_CHUNK_SIZE : size;
    qypk.size=len;
    
    int i;
    for(i=0;i<len;i++) qypk.source[i]=data[i];
    
    wristr(mypassword,qypk.header.password);
    
    if(iscrypt)crypt_encode(&qypk,&key);
    
    for(i=0;i<8;i++)
      send(parIP,parPort,&qypk,sizeof(qypk));
  }
  void live(netSource * qypk){
    if(config::nodemode){
      server.livesrc(qypk,1);
      return ;
    }
    
    uint32_t  len;
    int i;
    
    qypk->header.mode=_LIVESRC;
    qypk->header.userid=myuserid;
    
    int rdn=randnum();
    qypk->header.unique=rdn;
    
    wristr(mypassword,qypk->header.password);
    
    if(iscrypt)crypt_encode(qypk,&key);
    
    for(i=0;i<8;i++)
      send(parIP,parPort,&qypk,sizeof(qypk));
  }
  void liveclient(bool(*callback)(void*,int,void*),void * arg){
    netSource buf;
    in_addr   from;
    short     port;
    while(liveclientrunning){
      if(!wait_for_data(1,0))continue;
      bzero(&buf,sizeof(buf));
      if(recv(&from,&port,&buf,sizeof(netSource))){
        if(from.s_addr==parIP.s_addr && port==parPort){
          if(!ysDB.logunique(buf.header.userid(),buf.header.unique)) continue;
          if(buf.header.mode!=_LIVE)continue;
          if(config::autoboardcast){
            live(&buf);
          }
          if(!callback(&(buf.source),buf.size(),arg))return;
        }
      }
    }
  }
  bool updatekey(){
    netSource qypk;
    netSource buf;
    int i;
    in_addr  from;
    short    port;
    bzero(&qypk,sizeof(qypk));
    bzero(&buf ,sizeof(buf ));
    Key senddata;
    senddata.buf=(Key::netSendkey*)&(qypk.source);
    senddata.initbuf();
    qypk.header.mode=_UPDATEKEY;
    qypk.header.userid=myuserid;
    
    int rdn=randnum();
    qypk.header.unique=rdn;
    
    qypk.size=sizeof(Key::netSendkey);
    wristr(mypassword,qypk.header.password);
    if(iscrypt)crypt_encode(&qypk,&key);
    for(i=0;i<10;i++){
      send(parIP,parPort,&qypk,sizeof(qypk));
      uploop2:
      if(recv_within_time(&from,&port,&buf,sizeof(buf),1,0)){
        if(from.s_addr==parIP.s_addr && port==parPort){
          crypt_decode(&buf,&key);
          
          if(rdn!=qypk.header.unique) goto uploop2;
          
          if(config::checkSign)
          if(!senddata.checksign())return 0;
          senddata.computekey((unsigned char*)&buf.source);
          for(i=0;i<16;i++){
            this->key.data[i]=senddata.shared[i];
          }
          return 1;
        }
        else
          goto uploop2;
      }
    }
    return 0;
  }
  bool connectToUser(int32_t uid,in_addr * oaddr,short * oport){
    netQuery qypk;
    netQuery buf;
    int      i;
    in_addr  from;
    short    port;
    bzero(&qypk,sizeof(qypk));
    bzero(&buf ,sizeof(buf ));
    qypk.header.mode=_CONNECTUSER;
    wristr(mypassword,qypk.header.password);
    
    int rdn=randnum();
    qypk.header.unique=rdn;
    
    qypk.header.userid=myuserid;
    qypk.header.globalMode=globalmode;
    qypk.num1=uid;
    if(iscrypt)crypt_encode(&qypk,&key);
    for(i=0;i<10;i++){
      send(parIP,parPort,&qypk,sizeof(qypk));
      dsloop1:
      if(recv_within_time(&from,&port,&buf,sizeof(buf),1,0)){
        if(from.s_addr==parIP.s_addr && port==parPort){
          crypt_decode(&buf,&key);
          if(buf.header.mode==_P2PCONNECT){
            *oport =buf.num1();
            *oaddr =buf.addr;
            if(p2pconnect(buf.addr,buf.num1()))
              return 1;
            else
              return 0;
          }else
            return 0;
        }
        else
          goto dsloop1;
      }
    }
  }
  bool newSrc(const char * sname){
    netQuery qypk;
    netQuery buf;
    int      i;
    in_addr  from;
    short    port;
    bzero(&qypk,sizeof(qypk));
    bzero(&buf ,sizeof(buf ));
    qypk.header.mode=_NEWSRC;
    wristr(mypassword,qypk.header.password);
    for(i=0;i<8;i++)
      qypk.str1[i]=sname[i];
    
    int rdn=randnum();
    qypk.header.unique=rdn;
    
    qypk.header.userid=myuserid;
    qypk.header.globalMode=globalmode;
    if(iscrypt)crypt_encode(&qypk,&key);
    for(i=0;i<10;i++){
      send(parIP,parPort,&qypk,sizeof(qypk));
      dsloop1:
      if(recv_within_time(&from,&port,&buf,sizeof(buf),1,0)){
        if(from.s_addr==parIP.s_addr && port==parPort){
          crypt_decode(&buf,&key);
          
          if(rdn!=qypk.header.unique) goto dsloop1;
          
          if(buf.header.mode==_SUCCEED)
            return 1;
          else
            return 0;
        }
        else
          goto dsloop1;
      }
    }
  }
  bool downloadchunk(const char * sname,FILE * path,int32_t s){
    netQuery  qypk;
    netSource buf;
    int      i;
    in_addr  from;
    short    port;
    bzero(&qypk,sizeof(qypk));
    bzero(&buf ,sizeof(buf ));
    qypk.header.mode=_GETSRC;
    qypk.header.userid=myuserid;
    qypk.num1=s;
    
    int rdn=randnum();
    qypk.header.unique=rdn;
    
    for(i=0;i<8;i++)
      qypk.str1[i]=sname[i];
    qypk.header.globalMode=globalmode;
    wristr(mypassword,qypk.header.password);
    if(iscrypt)crypt_encode(&qypk,&key);
    for(i=0;i<10;i++){
      send(parIP,parPort,&qypk,sizeof(qypk));
      dcloop1:
      if(recv_within_time(&from,&port,&buf,sizeof(buf),1,0)){
        if(from.s_addr==parIP.s_addr && port==parPort){
          crypt_decode(&buf,&key);
          
          if(rdn!=qypk.header.unique) goto dcloop1;
          
          if(buf.header.mode==_SETSRC_APPEND){
            int size;
            size=buf.size()>SOURCE_CHUNK_SIZE ? SOURCE_CHUNK_SIZE : buf.size();
            size=size>0 ? size : 0;
            fwrite(buf.source,size,1,path);
            if(buf.size()==SOURCE_CHUNK_SIZE)
              return 1;
            else
              return 0;
          }else
            return 0;
        }
        else
          goto dcloop1;
      }
    }
    return 0;
  }
  void download(const char * sname,const char * path){
    FILE * f=fopen(path,"a");
    int i=0;
    while(downloadchunk(sname,f,i))i++;
    fclose(f);
  }
  bool uploadchunk(const char *sname,const char *buf2,int32_t size){
    netSource qypk;
    netQuery  buf;
    int      i;
    in_addr  from;
    short    port;
    bzero(&qypk,sizeof(qypk));
    bzero(&buf ,sizeof(buf ));
    qypk.header.mode=_SETSRC_APPEND;
    qypk.header.userid=myuserid;
    //int r=1;
    
    int rdn=randnum();
    qypk.header.unique=rdn;
    
    qypk.size=size;
    for(i=0;i<8;i++)
      qypk.title[i]=sname[i];
    qypk.header.globalMode=globalmode;
    wristr(mypassword,qypk.header.password);
    for(i=0;(i<size && i<SOURCE_CHUNK_SIZE);i++){
      qypk.source[i]=buf2[i];
    }
    if(iscrypt)crypt_encode(&qypk,&key);
    for(i=0;i<10;i++){
      send(parIP,parPort,&qypk,sizeof(qypk));
      ucloop2:
      if(recv_within_time(&from,&port,&buf,sizeof(buf),1,0)){
        if(from.s_addr==parIP.s_addr && port==parPort){
          crypt_decode(&buf,&key);
          
          if(rdn!=qypk.header.unique) goto ucloop2;
          
          if(buf.header.mode==_SUCCEED){
            return 1;
          }else
            return 0;
        }
        else
          goto ucloop2;
      }
    }
    return 0;
  }
  void upload(const char * sname,const char * path){
    int i;
    char buf[SOURCE_CHUNK_SIZE];
    FILE * ff=fopen(path,"r");
    while(!feof(ff)){
      for(i=0;(i<SOURCE_CHUNK_SIZE && !feof(ff));i++)
        buf[i]=fgetc(ff);
      if(!uploadchunk(sname,buf,i))return;
    }
    fclose(ff);
  }
  bool del(char * sname){
    netQuery qypk;
    netQuery buf;
    int      i;
    in_addr  from;
    short    port;
    bzero(&qypk,sizeof(qypk));
    bzero(&buf ,sizeof(buf ));
    qypk.header.mode=_DELSRC;
    for(i=0;i<8;i++)
      qypk.str1[i]=sname[i];
    wristr(mypassword,qypk.header.password);
    
    int rdn=randnum();
    qypk.header.unique=rdn;
    
    qypk.header.userid=myuserid;
    qypk.header.globalMode=globalmode;
    if(iscrypt)crypt_encode(&qypk,&key);
    for(i=0;i<10;i++){
      send(parIP,parPort,&qypk,sizeof(qypk));
      dsloop1:
      if(recv_within_time(&from,&port,&buf,sizeof(buf),1,0)){
        if(from.s_addr==parIP.s_addr && port==parPort){
          crypt_decode(&buf,&key);
          
          if(rdn!=qypk.header.unique) goto dsloop1;
          
          if(buf.header.mode==_SUCCEED)
            return 1;
          else
            return 0;
        }
        else
          goto dsloop1;
      }
    }
  }
  bool setPwd(const char * pwd,const char * lpwd){
    netQuery qypk;
    netQuery buf;
    int      i;
    in_addr  from;
    short    port;
    bzero(&qypk,sizeof(qypk));
    bzero(&buf ,sizeof(buf ));
    qypk.header.mode=_SETPWD;
    wristr(mypassword,qypk.header.password);
    wristr(pwd       ,qypk.str1);
    wristr(lpwd      ,qypk.str2);
    
    int rdn=randnum();
    qypk.header.unique=rdn;
    
    qypk.header.userid=myuserid;
    if(iscrypt)crypt_encode(&qypk,&key);
    for(i=0;i<10;i++){
      send(parIP,parPort,&qypk,sizeof(qypk));
      dsloop1:
      if(recv_within_time(&from,&port,&buf,sizeof(buf),1,0)){
        if(from.s_addr==parIP.s_addr && port==parPort){
          crypt_decode(&buf,&key);
          
          if(rdn!=qypk.header.unique) goto dsloop1;
          
          if(buf.header.mode==_SUCCEED)
            return 1;
          else
            return 0;
        }
        else
          goto dsloop1;
      }
    }
  }
  bool setPwd(const char * pwd,int32_t puserid){
    netQuery qypk;
    netQuery buf;
    int      i;
    in_addr  from;
    short    port;
    bzero(&qypk,sizeof(qypk));
    bzero(&buf ,sizeof(buf ));
    qypk.header.mode=_SETPWD;
    wristr(mypassword,qypk.header.password);
    wristr(pwd       ,qypk.str1);
    
    int rdn=randnum();
    qypk.header.unique=rdn;
    
    qypk.header.userid=myuserid;
    qypk.header.globalMode='t';
    qypk.num1=puserid;
    if(iscrypt)crypt_encode(&qypk,&key);
    for(i=0;i<10;i++){
      send(parIP,parPort,&qypk,sizeof(qypk));
      dsloop1:
      if(recv_within_time(&from,&port,&buf,sizeof(buf),1,0)){
        if(from.s_addr==parIP.s_addr && port==parPort){
          crypt_decode(&buf,&key);
          
          if(rdn!=qypk.header.unique) goto dsloop1;
          
          if(buf.header.mode==_SUCCEED)
            return 1;
          else
            return 0;
        }
        else
          goto dsloop1;
      }
    }
  }
  bool newUser(int32_t uid){
    netQuery qypk;
    netQuery buf;
    int      i;
    in_addr  from;
    short    port;
    bzero(&qypk,sizeof(qypk));
    bzero(&buf ,sizeof(buf ));
    qypk.header.mode=_NEWUSER;
    qypk.num1=uid;
    wristr(mypassword,qypk.header.password);
    
    int rdn=randnum();
    qypk.header.unique=rdn;
    
    qypk.header.userid=myuserid;
    qypk.header.globalMode='t';
    if(iscrypt)crypt_encode(&qypk,&key);
    for(i=0;i<10;i++){
      send(parIP,parPort,&qypk,sizeof(qypk));
      dsloop1:
      if(recv_within_time(&from,&port,&buf,sizeof(buf),1,0)){
        if(from.s_addr==parIP.s_addr && port==parPort){
          crypt_decode(&buf,&key);
          
          if(rdn!=qypk.header.unique) goto dsloop1;
          
          if(buf.header.mode==_SUCCEED)
            return 1;
          else
            return 0;
        }
        else
          goto dsloop1;
      }
    }
  }
}client(CLIENT_PORT);
}
#endif