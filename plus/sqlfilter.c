#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include "lua/src/lua.h"
#include "lua/src/lualib.h"
#include "lua/src/lauxlib.h"
#include "lua/src/luaconf.h"
char reps[]={'\'','"','%','\0'};
int sqlfilter(lua_State * L){
  const char * istr;
  char *ostr,*sp2;
  const char * sp;
  int len,i;
  if(!lua_isstring(L,1))return 0;
  istr=lua_tostring(L,1);
  len=strlen(istr);
  ostr=(char*)malloc(len*2 + 1);
  sp2=ostr;
  for(i=0;i<len;i++){
    sp=reps;
    while(*sp){
      if(istr[i]==*sp){
        *sp2='\\';
        sp2++;
        goto end;
      }
      sp++;
    }
    end:
    *sp2=istr[i];
    sp2++;
  }
  *sp2='\0';
  lua_pushstring(L,ostr);
  return 1;
  free(ostr);
}
void lua_open(lua_State * L){
  lua_register(L,"sqlfilter",sqlfilter);
}