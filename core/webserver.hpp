#ifndef yrssf_core_webserver
#define yrssf_core_webserver
#define WEB_FLAG_DEF        (lwan_handler_flags_t)(HANDLER_PARSE_COOKIES|HANDLER_PARSE_QUERY_STRING|HANDLER_PARSE_POST_DATA)
#define WEB_FLAG_UPL        (lwan_handler_flags_t)(HANDLER_PARSE_COOKIES|HANDLER_PARSE_QUERY_STRING)
#include "client.hpp"
extern "C"{
#include "lwan.h"
#include "lwan-serve-files.h"
#include "zlib.h"
}
static lwan_http_status_t ajax(lwan_request_t *request,lwan_response_t *response, void *data){
    int i,l;
    lua_State * L=lua_newthread(yrssf::gblua);
    lwan_key_value_t *headers = (lwan_key_value_t*)coro_malloc(request->conn->coro, sizeof(*headers) * 2);
    if (UNLIKELY(!headers))
        return HTTP_INTERNAL_ERROR;
    headers[0].key   = (char*)"Cache-Control";
    headers[0].value = (char*)"no-cache";
    headers[1].key   = NULL;
    headers[1].value = NULL;
    char buf[1025];
    l=request->url.len>1024?1024:request->url.len;
    for(i=0;i<l;i++){
      buf[i]=request->url.value[i];
    }
    buf[i+1]='\0';
    lua_pushstring(L,buf);
    lua_setglobal(L,"URL");
    l=request->original_url.len>1024?1024:request->original_url.len;
    for(i=0;i<l;i++){
      buf[i]=request->original_url.value[i];
    }
    buf[i+1]='\0';
    lua_pushstring(L,buf);
    lua_setglobal(L,"ORIGINAL_URL");
    const char * message;
    const static char Emessage[]="NULL";
    response->mime_type = "text/html;charset=utf-8";
    response->headers   = headers;
    lua_createtable(L,0,request->query_params.len);
    for(i=0;i<request->query_params.len;i++){
      lua_pushstring(L, request->query_params.base[i].key);
      lua_pushstring(L, request->query_params.base[i].value);
      lua_settable(L, -3);
    }
    lua_setglobal(L,"GET");
    lua_createtable(L,0,request->post_data.len);
    for(i=0;i<request->post_data.len;i++){
      lua_pushstring(L, request->post_data.base[i].key);
      lua_pushstring(L, request->post_data.base[i].value);
      lua_settable(L, -3);
    }
    lua_setglobal(L,"POST");
    lua_createtable(L,0,request->cookies.len);
    for(i=0;i<request->cookies.len;i++){
      lua_pushstring(L, request->cookies.base[i].key);
      lua_pushstring(L, request->cookies.base[i].value);
      lua_settable(L, -3);
    }
    lua_setglobal(L,"COOKIE");
    luaL_dofile(L,"./ajax.lua");
    if(lua_isstring(L,-1)){
      std::cout<<lua_tostring(L,-1)<<std::endl;
    }
    lua_getglobal(L,"RESULT");
    if(lua_isstring(L,-1)){
      message=lua_tostring(L,-1);
      l=strlen(message);
      strbuf_set(response->buffer, message,l);
      return HTTP_OK;
    }else{
      l=sizeof(Emessage);
      strbuf_set_static(response->buffer,Emessage,l-1);
      return HTTP_OK;
    }
}
static lwan_http_status_t upload_b(lwan_request_t *request,lwan_response_t *response, void *data,const char * luapath,bool unzip){
    int  fd;
    int l,i;
    static int cnum=0;
    cnum++;
    char path[PATH_MAX];
    const static char Emessage[]="<error>NULL</error>";
    const char * message;
    if(UNLIKELY(request->header.body==NULL)){
      //ysDebug("err");
      return HTTP_INTERNAL_ERROR;
    }
    sprintf(
      path,
      "/tmp/yrssf_upload_%d_%d",
      yrssf::randnum(),
      cnum
    );
    char * value=request->header.body->value;
    unsigned int len=request->header.body->len;
    bool iszip=0;
    fd=open(path,O_RDWR|O_CREAT);
    //zip
    if(unzip && len>4){
      int b1=value[0];
      int b2=value[1];
      if(((b1 & 0xff)|((b2 & 0xff)<<8))==0x8b1f){
        iszip=1;
      }
    }
    write(
      fd,
      value,
      len
    );
    close(fd);
    lua_State * L=lua_newthread(yrssf::gblua);
    lua_pushstring(L,path);
    lua_setglobal(L,"FILE_PATH");
    lua_pushboolean(L,iszip);
    lua_setglobal(L,"FILE_IS_ZIP");
    lua_createtable(L,0,request->cookies.len);
    for(i=0;i<request->cookies.len;i++){
      lua_pushstring(L, request->cookies.base[i].key);
      lua_pushstring(L, request->cookies.base[i].value);
      lua_settable(L, -3);
    }
    lua_setglobal(L,"COOKIE");
    lua_createtable(L,0,request->query_params.len);
    for(i=0;i<request->query_params.len;i++){
      lua_pushstring(L, request->query_params.base[i].key);
      lua_pushstring(L, request->query_params.base[i].value);
      lua_settable(L, -3);
    }
    lua_setglobal(L,"GET");
    luaL_dofile(L,luapath);
    remove(path);
    response->mime_type = "text/xml;charset=utf-8";
    if(lua_isstring(L,-1)){
      std::cout<<lua_tostring(L,-1)<<std::endl;
    }
    lua_getglobal(L,"RESULT");
    if(lua_isstring(L,-1)){
      message=lua_tostring(L,-1);
      l=strlen(message);
      strbuf_set(response->buffer, message,l);
      return HTTP_OK;
    }else{
      l=sizeof(Emessage);
      strbuf_set_static(response->buffer,Emessage,l-1);
      return HTTP_OK;
    }
}
static lwan_http_status_t xmlRPC(lwan_request_t *request,lwan_response_t *response, void *data){
    return upload_b(request,response,data,"xmlRPC.lua",1);
}
static lwan_http_status_t uploader(lwan_request_t *request,lwan_response_t *response, void *data){
    return upload_b(request,response,data,"uploader.lua",0);
}
void webserverRun(){
    struct lwan_serve_files_settings_t sfile;
    bzero(&sfile,sizeof(sfile));
    sfile.root_path                        = "static";
    sfile.index_html                       = "index.html";
    sfile.serve_precompressed_files        = true;
    sfile.directory_list_template          = NULL;
    sfile.auto_index                       = true;
    
    lwan_url_map_t default_map[]={
    //handler	data	prefix		len	flags				module				args		realm	password_file
      ajax	,NULL	,"/ajax"	,0	,WEB_FLAG_DEF			,NULL				,NULL	,{	NULL	,NULL	},
      xmlRPC	,NULL	,"/wmexam"	,0	,WEB_FLAG_UPL			,NULL				,NULL	,{	NULL	,NULL	},
      uploader	,NULL	,"/PutTemproraryStorage",0,WEB_FLAG_UPL			,NULL				,NULL	,{	NULL	,NULL	},
      uploader	,NULL	,"/uploader"	,0	,WEB_FLAG_UPL			,NULL				,NULL	,{	NULL	,NULL	},
      NULL	,NULL	,"/"		,0	,(lwan_handler_flags_t)0	,lwan_module_serve_files()	,&sfile	,{	NULL	,NULL	},
      NULL	,NULL	,NULL		,0	,(lwan_handler_flags_t)0	,NULL				,NULL	,{	NULL	,NULL	}
    };
    
    lwan_t l;
    lwan_init(&l);
    lwan_set_url_map(&l, default_map);
    
    lwan_main_loop(&l);
    lwan_status_debug("\033[40;43mYRSSF:\033[0m\033[40;37mYRSSF shutdown\033[0m\n");
    
    default_map[0].prefix  = NULL;
    default_map[0].handler = NULL;
    default_map[1].prefix  = NULL;
    default_map[1].module  = NULL;
    default_map[1].flags   = (lwan_handler_flags_t)0;
    default_map[1].args    = NULL;
    
    
    lwan_status_debug("\033[40;43mYRSSF:\033[0m\033[40;37mweb server shutdown\033[0m\n");
    lwan_shutdown(&l);
    lwan_status_debug("\033[40;43mYRSSF:\033[0m\033[40;37mweb shutdown success\033[0m\n");
}
#endif