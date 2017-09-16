#ifndef yrssf_core_webserver
#define yrssf_core_webserver
#include "client.hpp"
#include "httpd.hpp"
#include "global.hpp"
#include "websocket.hpp"
void ajax(yrssf::httpd::request * req){
    auto Lp=yrssf::luapool::Create();
    lua_State * L=Lp->L;
    
    char buf[4096];
    
    //ysDebug("debug");
    req->init();
    //ysDebug("debug");
    req->query_decode();
    //ysDebug("debug");
    req->getcookie();
    req->cookie_decode();
    //yrssf::httpd::readBufferBeforeSend(req->fd);
    
    std::map<std::string,std::string>::iterator it;
    
    lua_createtable(L,0,req->paseredquery.size());
    for(
      it =req->paseredquery.begin();
      it!=req->paseredquery.end();
      it++
    ){
      lua_pushstring(L, it->first.c_str());
      strcpy(buf,it->second.c_str());
      yrssf::url_decode(buf,strlen(buf));
      lua_pushstring(L,buf);
      lua_settable(L, -3);
    }
    lua_setglobal(L,"GET");
    
    if(req->postmode){
      req->getpost();
      //ysDebug("debug");
      req->post_decode();
      lua_createtable(L,0,req->paseredpost.size());
      for(
        it =req->paseredpost.begin();
        it!=req->paseredpost.end();
        it++
      ){
        lua_pushstring(L, it->first.c_str());
        strcpy(buf,it->second.c_str());
        yrssf::url_decode(buf,strlen(buf));
        lua_pushstring(L,buf);
        lua_settable(L, -3);
      }
      lua_setglobal(L,"POST");
    }
    
    lua_createtable(L,0,req->paseredcookie.size());
    for(
      it =req->paseredcookie.begin();
      it!=req->paseredcookie.end();
      it++
    ){
      lua_pushstring(L, it->first.c_str());
      lua_pushstring(L, it->second.c_str());
      lua_settable(L, -3);
    }
    lua_setglobal(L,"COOKIE");
    
    lua_createtable(L,0,req->paseredheader.size());
    for(
      it =req->paseredheader.begin();
      it!=req->paseredheader.end();
      it++
    ){
      lua_pushstring(L, it->first.c_str());
      lua_pushstring(L, it->second.c_str());
      lua_settable(L, -3);
    }
    lua_setglobal(L,"HEADER");
    
    //ysDebug("call lua");
    luaL_dofile(L,"./ajax.lua");
    if(lua_isstring(L,-1)){
      std::cout<<lua_tostring(L,-1)<<std::endl;
    }
    
    yrssf::httpd::writeStr(req->fd,
      "HTTP/1.0 200 OK\r\n"
      "Server:yrssf-ajax-server\r\n"
      "Content-Type:text/html;charset=UTF-8\r\n"
      "Cache-Control:no-cache\r\n"
      "\r\n"
    );
    lua_getglobal(L,"RESULT");
    if(lua_isstring(L,-1)){
      yrssf::httpd::writeStr(req->fd,lua_tostring(L,-1));
    }else{
      yrssf::httpd::writeStr(req->fd,"NULL");
    }
    yrssf::luapool::Delete(Lp);
}
void msrc(yrssf::httpd::request * req){
    req->init();
    //ysDebug("debug");
    req->query_decode();
    //ysDebug("debug");
    req->getcookie();
    req->cookie_decode();
    
    //yrssf::httpd::readBufferBeforeSend(req->fd);
    
    std::string url="../";
    url+=yrssf::client.parhash;
    
    const char * tmp=strrchr(req->url,'/');
    
    if(tmp){
      tmp++;
      if(*tmp){
        url+="/";
        url+=tmp;
      }
    }
    
    auto loca=std::string("Location:")+url+"\r\n";
    
    yrssf::httpd::writeStr(req->fd,
      "HTTP/1.0 302 Moved Temporarily\r\n"
      "Server:yrssf-source-server\r\n"
    );
    yrssf::httpd::writeStr(req->fd,loca.c_str());
    yrssf::httpd::writeStr(req->fd,"\r\n\r\n");
}
void webserverRun(){
    yrssf::httpd::addrule(ajax,"/ajax");
    yrssf::httpd::addrule(msrc,"/mysource");
    yrssf::httpd::addlongrule(yrssf::websocket::callback,"/websocket");
    yrssf::httpd::run(yrssf::config::L.httpdPort);
}
#endif