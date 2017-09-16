#ifndef yrssf_core_client
#define yrssf_core_client
#include "server.hpp"
#include "global.hpp"
#include "func.hpp"
namespace yrssf{
class Client:public Server{
  public:
  bool liveclientrunning;
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
  void liveBegin(){
    netQuery  qypk;
    
    qypk.header.mode=_LIVE_B;
    qypk.header.userid=myuserid;
    
    int rdn=randnum();
    qypk.header.unique=rdn;
    
    wristr(mypassword,qypk.header.password);
    
    if(iscrypt)crypt_encode(&qypk,&key);
    
    for(int i=0;i<8;i++)
      send(parIP,parPort,&qypk,sizeof(qypk));
  }
  void liveEnd(){
    netQuery  qypk;
    
    qypk.header.mode=_LIVE_E;
    qypk.header.userid=myuserid;
    
    int rdn=randnum();
    qypk.header.unique=rdn;
    
    wristr(mypassword,qypk.header.password);
    
    if(iscrypt)crypt_encode(&qypk,&key);
    
    for(int i=0;i<8;i++)
      send(parIP,parPort,&qypk,sizeof(qypk));
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
    //认证。
    //用于服务器没有登记客户端帐号的情况下
    //由受信任的服务器开证明
    //强行创建帐号
    //
    //先用getUniKey开证明
    //然后连接到服务器
    //再调用此方法
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
    //wristr(mypassword,qypk.header.password);
    //认证，密码在证书里面
    if(!getPbk())return 0;
    
    auto tR=(netReg*)respk.source;
    memcpy(tR->key,parpbk,ECDH_SIZE);
    tR->encrypt();
    //对数据进行非对称加密
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
    liveBegin();
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
    liveEnd();
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
  int callPlus(lua_State * L){
    netQuery qypk;
    netQuery buf;
    int      i;
    in_addr  from;
    short    port;
    
    char     bufs[17];
    bufs[16]='\0';
      
    bzero(&qypk,sizeof(qypk));
    bzero(&buf ,sizeof(buf ));
    qypk.header.mode=_PLUS;
    wristr(mypassword,qypk.header.password);
    
    int rdn=randnum();
    qypk.header.unique=rdn;
    
    qypk.header.userid=myuserid;
    
      //从这里可以看出，callPlus的数据包非常小
      //要传输文件，请直接上传资源，然后用callPlus传资源id
      if(lua_isstring(L,1))
        wristr(lua_tostring(L,1),qypk.str1);
      if(lua_isstring(L,2))
        wristr(lua_tostring(L,2),qypk.str2);
      if(lua_isinteger(L,3))
        qypk.num1=lua_tointeger(L,3);
      if(lua_isinteger(L,4))
        qypk.num2=lua_tointeger(L,4);
      if(lua_isinteger(L,5))
        qypk.num3=lua_tointeger(L,5);
      if(lua_isinteger(L,6))
        qypk.num4=lua_tointeger(L,6);
      
    
    if(iscrypt)crypt_encode(&qypk,&key);
    for(i=0;i<10;i++){
      send(parIP,parPort,&qypk,sizeof(qypk));
      dsloop1:
      if(recv_within_time(&from,&port,&buf,sizeof(buf),1,0)){
        if(from.s_addr==parIP.s_addr && port==parPort){
          crypt_decode(&buf,&key);
          
          if(rdn!=qypk.header.unique) goto dsloop1;
          
          if(buf.header.mode==_PLUS){
            
            //push querys
            wristr(buf.str1, bufs);
            lua_pushstring(L,bufs);
            
            wristr(buf.str2, bufs);
            lua_pushstring(L,bufs);
            
            lua_pushinteger(L,buf.num1());
            lua_pushinteger(L,buf.num2());
            lua_pushinteger(L,buf.num3());
            lua_pushinteger(L,buf.num4());
        
            return 6;
          }else
            return 0;
        }
        else
          goto dsloop1;
      }
    }
  }
}client(config::L.yscPort);
}
#endif