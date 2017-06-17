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
#define SERVER_PORT         1215
#define CLIENT_PORT         8001
#define SOURCE_CHUNK_SIZE   4096

#define ysDebug(fmt, ...) printf("debug: %s:%d %s() " fmt "\n",__FILE__, __LINE__, __FUNCTION__,##__VA_ARGS__);
#define ysError(fmt, ...) printf("error: %s:%d %s() " fmt "\n",__FILE__, __LINE__, __FUNCTION__,##__VA_ARGS__);

namespace yrssf{
  namespace config{
    bool AllowShell   =0;
    bool checkSign    =0;
    bool nodemode     =0;
    bool stop         =0;
    bool fastcgi      =0;
    bool autoboardcast=0;
    bool liveputout   =1;
    double livepresent=0.01d;
    unsigned int httpBodyLength=65536;
    unsigned int maxRequest=16;
  }
  //lua_State * gblua;
  std::mutex clientlocker;
  bool clientdisabled=0;
}
#endif