#ifndef yrssf_core_global
#define yrssf_core_global
extern "C" {
#include "lua/src/lua.h"
#include "lua/src/lualib.h"
#include "lua/src/lauxlib.h"
#include "lua/src/luaconf.h"
}
#include <mutex>
#define SERVER_PORT         1215
#define CLIENT_PORT         8001
#define SOURCE_CHUNK_SIZE   4096
#define WEB_FLAG_DEF        (lwan_handler_flags_t)(HANDLER_PARSE_COOKIES|HANDLER_PARSE_QUERY_STRING|HANDLER_PARSE_POST_DATA)
#define WEB_FLAG_UPL        (lwan_handler_flags_t)(HANDLER_PARSE_COOKIES|HANDLER_PARSE_QUERY_STRING)
#define ysDebug(fmt, ...) \
  printf("status %s:%d %s() ",__FILE__, __LINE__, __FUNCTION__); \
  printf(fmt, ##__VA_ARGS__); \
  printf("\n");
#define ysError(fmt, ...) \
  printf("error  %s:%d %s() ",__FILE__, __LINE__, __FUNCTION__); \
  printf(fmt, ##__VA_ARGS__); \
  printf("\n");
namespace yrssf{
namespace config{
  bool AllowShell=0;
  bool checkSign=0;
}
lua_State * gblua;
std::mutex clientlocker;
bool clientdisabled=0;
}
#endif