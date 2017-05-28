#ifndef yrssf_core_webserver
#define yrssf_core_webserver
#include "client.hpp"
#include "httpd.hpp"
void ajax(yrssf::httpd::request * req){
    lua_State * L=lua_newthread(yrssf::gblua);
    
    char buf[4096];
    
    //ysDebug("debug");
    req->init();
    //ysDebug("debug");
    req->query_decode();
    //ysDebug("debug");
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
}
void webserverRun(){
    yrssf::httpd::addrule(ajax,"/ajax");
    yrssf::httpd::run(1215);
}
#endif