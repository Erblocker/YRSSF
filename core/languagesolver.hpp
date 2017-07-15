#ifndef yrssf_languageresolver
#define yrssf_languageresolver
#include "global.hpp"
#include "cppjieba/Jieba.hpp"
#include "rwmutex.hpp"
extern "C" {
#include "lua/src/lua.h"
#include "lua/src/lualib.h"
#include "lua/src/lauxlib.h"
#include "lua/src/luaconf.h"
}
namespace yrssf{
  namespace langsolver{
    RWMutex locker;
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
      //std::string lang_string;
      int n=1;
      if(!lua_isstring(L,1)) return 0;
      //lang_string=lua_tostring(L,1);
      //ysDebug("solve");
      solver->Tag(lua_tostring(L,1),lang_tagres);
      //ysDebug("solve");
      lua_newtable(L);//create main array
      lua_pushnil(L);
      lua_rawseti(L,-2,0);
      for(auto it=lang_tagres.begin();it!=lang_tagres.end();it++){
        lua_newtable(L);//create child array
        //lua_pushnumber(L,n);
        lua_pushnil(L);
        lua_rawseti(L,-2,0);//put id into child array
        lua_pushstring(L,it->first.c_str());
        lua_rawseti(L,-2,1);//put data into child array
        lua_pushstring(L,it->second.c_str());
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
      solver->extractor.Extract(lua_tostring(L,1),keywordres, topk);
      lua_newtable(L);//create main array
      lua_pushnil(L);
      lua_rawseti(L,-2,0);
      int n=1,j;
      for(it=keywordres.begin();it!=keywordres.end();it++){
        //cppjieba::KeywordExtractor::Word &lp=*it;
        lua_newtable(L);//create child array
        //lua_pushnumber(L,n);
        lua_pushnil(L);
        lua_rawseti(L,-2,0);//put id into child array
        lua_pushstring(L,it->word.c_str());
        lua_rawseti(L,-2,1);//put data into child array
        lua_pushnumber(L,it->weight);
        lua_rawseti(L,-2,2);//put data into child array
          
          lua_newtable(L);//create a array for offset
            //lua_pushnumber(L,-1);
            lua_pushnil(L);
            lua_rawseti(L,-2,0);
            j=1;
            std::vector<std::size_t> &offset=it->offsets;
            auto oit=offset.begin();
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
      if(solver)return 0;
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
      if(!solver)return 0;
      delete solver;
      solver=NULL;
      return 0;
    }
    int lang_insert(lua_State * L){
      if(!solver)return 0;
      if(!lua_isstring(L,1)) return 0;
      bool res;
      if(lua_isstring(L,2)){
        res=solver->InsertUserWord(std::string(lua_tostring(L,1)),std::string(lua_tostring(L,2)));
      }else{
        res=solver->InsertUserWord(std::string(lua_tostring(L,1)));
      }
      lua_pushboolean(L,res);
      return 1;
    }
    luaL_Reg reg[]={
      {"init",[](lua_State * L){
          locker.Wlock();
          int r=lang_init(L);
          locker.unlock();
          return r;
        }
      },
      {"destory",[](lua_State * L){
          locker.Wlock();
          int r=lang_destory(L);
          locker.unlock();
          return r;
        }
      },
      {"insert",[](lua_State * L){
          locker.Wlock();
          int r=lang_insert(L);
          locker.unlock();
          return r;
        }
      },
      {"solve",[](lua_State * L){
          locker.Rlock();
          int r=lang_solve(L);
          locker.unlock();
          return r;
        }
      },
      {"keyword",[](lua_State * L){
          locker.Rlock();
          int r=lang_keyword(L);
          locker.unlock();
          return r;
        }
      },
      {"status",[](lua_State * L){
          locker.Rlock();
          if(solver)
            lua_pushboolean(L,1);
          else
            lua_pushboolean(L,0);
          locker.unlock();
          return 1;
        }
      },
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