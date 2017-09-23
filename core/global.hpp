#ifndef yrssf_core_global
#define yrssf_core_global
extern "C" {
#include "lua/src/lua.h"
#include "lua/src/lualib.h"
#include "lua/src/lauxlib.h"
#include "lua/src/luaconf.h"
}
#include "luapool.hpp"
#include <mutex>
#define SOURCE_CHUNK_SIZE   4096

#define ysDebug(fmt, ...) printf("debug: %s:%d %s() " fmt "\n",__FILE__, __LINE__, __FUNCTION__,##__VA_ARGS__);
#define ysError(fmt, ...) printf("error: %s:%d %s() " fmt "\n",__FILE__, __LINE__, __FUNCTION__,##__VA_ARGS__);

namespace yrssf{
  namespace config{
    bool AllowShell               =0;
    bool checkSign                =0;
    bool nodemode                 =0;
    bool stop                     =0;
    bool fastcgi                  =0;
    bool autoboardcast            =0;
    bool liveputout               =0;
    bool soundputout              =0;
    double livepresent            =0.01d;
    unsigned int httpBodyLength   =65536;
    unsigned int maxRequest       =16;
    class LL{
      public:
      short httpdPort;
      short ysPort;
      short yscPort;
      unsigned int tpoolsize;
      LL(){
        httpdPort=1215;
        ysPort=1215;
        yscPort=8000;
        tpoolsize=16;
        lua_State * lua=luaL_newstate();
        
        luaL_dofile(lua,"yrssf.conf");
        
        lua_getglobal(lua,"HTTPD_PORT");
        if(lua_isinteger(lua,-1)){
          httpdPort=lua_tointeger(lua,-1);
        }
        lua_pop(lua,1);
        
        lua_getglobal(lua,"YRSSF_PORT");
        if(lua_isinteger(lua,-1)){
          ysPort=lua_tointeger(lua,-1);
        }
        lua_pop(lua,1);
        
        lua_getglobal(lua,"CLIENT_PORT");
        if(lua_isinteger(lua,-1)){
          yscPort=lua_tointeger(lua,-1);
        }
        lua_pop(lua,1);
        
        lua_getglobal(lua,"THREAD");
        if(lua_isinteger(lua,-1)){
          tpoolsize=lua_tointeger(lua,-1);
        }
        lua_pop(lua,1);
        
        lua_close(lua);
      }
      ~LL(){}
    }L;
  }
  //lua_State * gblua;
  std::mutex clientlocker;
  bool clientdisabled=0;
}
#endif