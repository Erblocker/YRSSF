#include "../lua/src/lua.h"
#include "../lua/src/lualib.h"
#include "../lua/src/lauxlib.h"
#include "../lua/src/luaconf.h"
#include <stdio.h>
int init(lua_State * L){
	printf("libsuccess\0");
}