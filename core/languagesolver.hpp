#ifndef yrssf_languageresolver
#define yrssf_languageresolver
#include "cppjieba/Jieba.hpp"
extern "C" {
#include "lua/src/lua.h"
#include "lua/src/lualib.h"
#include "lua/src/lauxlib.h"
#include "lua/src/luaconf.h"
}
namespace yrssf{
  namespace langsolver{
    class Solver:public cppjieba::Jieba{
      public:
      Solver(
        const char* DICT_PATH,
        const char* HMM_PATH,
        const char* USER_DICT_PATH,
        const char* IDF_PATH,
        const char* STOP_WORD_PATH
      ):cppjieba::Jieba(DICT_PATH,
        HMM_PATH,
        USER_DICT_PATH,
        IDF_PATH,
        STOP_WORD_PATH){
          
        }
    }*solver=NULL;
    int lang_solve(lua_State * L){
      if(!solver)return 0;
      std::vector<std::pair<std::string,std::string> > lang_tagres;
      std::vector<std::pair<std::string,std::string> >::iterator it;
      //std::string lang_string;
      int n=1;
      std::pair<std::string,std::string> &lp=*it;
      if(!lua_isstring(L,1)) return 0;
      //lang_string=lua_tostring(L,1);
      solver->Tag(lua_tostring(L,1),lang_tagres);
      lua_newtable(L);//create main array
      lua_pushnil(L);
      lua_rawseti(L,-2,0);
      for(it=lang_tagres.begin();it!=lang_tagres.end();it++){
        lp=*it;
        lua_newtable(L);//create child array
        lua_pushnumber(L,n);
        lua_rawseti(L,-2,0);//put id into child array
        lua_pushstring(L,lp.first.c_str());
        lua_rawseti(L,-2,1);//put data into child array
        lua_pushstring(L,lp.second.c_str());
        lua_rawseti(L,-2,2);//put data into child array
        lua_rawseti(L,-2,n);//put child array into main array
        n++;
      }    
      return 1;
    }
    int lang_keyword(lua_State * L){
      if(!solver)return 0;
      if(!lua_isstring(L,1)) return 0;
      const std::size_t topk = 5;
      std::vector<cppjieba::KeywordExtractor::Word>         keywordres;
      std::vector<cppjieba::KeywordExtractor::Word>::iterator it;
      solver->extractor.Extract(lua_tostring(L,0),keywordres, topk);
      lua_newtable(L);//create main array
      lua_pushnil(L);
      lua_rawseti(L,-2,0);
      int n=1,j;
      for(it=keywordres.begin();it!=keywordres.end();it++){
        cppjieba::KeywordExtractor::Word &lp=*it;
        lua_newtable(L);//create child array
        lua_pushnumber(L,n);
        lua_rawseti(L,-2,0);//put id into child array
        lua_pushstring(L,lp.word.c_str());
        lua_rawseti(L,-2,1);//put data into child array
        lua_pushnumber(L,lp.weight);
        lua_rawseti(L,-2,2);//put data into child array
        lua_newtable(L);//create a array for offset
            lua_pushnumber(L,-1);
            lua_rawseti(L,-2,0);
            j=1;
            std::vector<std::size_t> &offset=lp.offsets;
            std::vector<std::size_t>::iterator oit=offset.begin();
            for(;oit!=offset.end();oit++){
                lua_pushnumber(L,*oit);
                lua_rawseti(L,-2,j);
                j++;
            }
        lua_rawseti(L,-2,3);
        lua_rawseti(L,-2,n);//put child array into main array
        n++;
      }
      return 1;
    }
    int lang_init(lua_State * L){
      if(!lua_isstring(L,1)) return 0;
      if(!lua_isstring(L,2)) return 0;
      if(!lua_isstring(L,3)) return 0;
      if(!lua_isstring(L,4)) return 0;
      if(!lua_isstring(L,5)) return 0;
      solver=new Solver(
        lua_tostring(L,1),
        lua_tostring(L,2),
        lua_tostring(L,3),
        lua_tostring(L,4),
        lua_tostring(L,5)
      );
      lua_pushboolean(L,1);
      return 1;
    }
    int lang_destory(lua_State * L){
      delete solver;
      solver=NULL;
    }
    luaL_Reg reg[]={
      {"init",    lang_init},
      {"destory", lang_destory},
      {"solve",   lang_solve},
      {"keyword", lang_keyword},
      {NULL,NULL}
    };
    int luaopen(lua_State * L){
      lua_newtable(L);
      luaL_setfuncs(L, reg, 0);
      lua_setglobal(L,"langsolver");
      return 0;
    }
  }
}
#endif