#ifndef yrssf_core_sta
#define yrssf_core_sta
#include "global.hpp"
#include <map>
#include <string>
#include <math.h>
namespace yrssf{
 namespace sta{
  int sta_num(lua_State * L){
    std::map<std::string,int>::iterator it;
    std::map<std::string,int>         data;
    if(!lua_istable(L,-1))return 0;
    int n = luaL_len(L,-1);
    for(int i=1;i<=n;i++){
        lua_pushnumber(L,i);
        lua_gettable(L,-2);
        if(!lua_isstring(L,-1))return 0;
        data[lua_tostring(L,-1)]++;
        lua_pop(L,-1);
    }
    n=1;
    lua_newtable(L);//create main array
    lua_pushnumber(L,-1);
    lua_rawseti(L,-2,0);  //fill array[0]
    for(it=data.begin();it!=data.end();it++){
        lua_newtable(L);
        lua_pushnumber(L,n);
        lua_rawseti(L,-2,0);
        lua_pushstring(L,it->first.c_str());
        lua_rawseti(L,-2,1);
        lua_pushnumber(L,it->second);
        lua_rawseti(L,-2,2);
        lua_rawseti(L,-2,n);
        n++;
    }
    return 1;
  }
  int sta_regression(lua_State * L){
    if(!lua_istable(L,1))return 0;
    //std::cout << "p1\0" << std::endl;
    int n = luaL_len(L,1);
    double x,y,X,Y,r,a,b;
    double xa=0,ya=0;
    double xa2=0,ya2=0;
    double xya=0;
    for(int i=1;i<=n;i++){
        lua_pushnumber(L,i);
        lua_gettable(L,1);
        if(!lua_istable(L,-1))return 0;
            //std::cout << "p2\0" << std::endl;
            lua_pushnumber(L,1);
            lua_gettable(L,-2);
            if(!lua_isnumber(L,-1))return 0;
            x=lua_tonumber(L,-1);
            lua_pop(L,1);
            lua_pushnumber(L,2);
            lua_gettable(L,-2);
            if(!lua_isnumber(L,-1))return 0;
            y=lua_tonumber(L,-1);
            lua_pop(L,1);
        lua_pop(L,1);
        xa+=x;
        ya+=y;
        xa2+=x*x;
        ya2+=y*y;
        xya+=x*y;
    }
    X=xa/n;
    Y=ya/n;
    b=(xya-(n*X*Y))/(xa2-(n*X*X));
    a=Y-b*X;
    r=(xya-(n*X*Y))/sqrt((xa2-(n*X*X))*(ya2-(n*Y*Y)));
    lua_pushnumber(L,xa);
    lua_pushnumber(L,ya);
    lua_pushnumber(L,X);
    lua_pushnumber(L,Y);
    lua_pushnumber(L,a);
    lua_pushnumber(L,b);
    lua_pushnumber(L,r);
    return 7;
  }
  int sta_independence(lua_State * L){
    if(!lua_isnumber(L,1))return 0;
    if(!lua_isnumber(L,2))return 0;
    if(!lua_isnumber(L,3))return 0;
    if(!lua_isnumber(L,4))return 0;
    double a=lua_tonumber(L,1);
    double b=lua_tonumber(L,2);
    double c=lua_tonumber(L,3);
    double d=lua_tonumber(L,4);
    double all,Pe;
    all=a+b+c+d;
    double k=(all*((a*d)-(b*c))*((a*d)-(b*c)))/((a+b)*(c+d)*(a+c)*(b+d));
    if(    k<0.455d             )Pe=0.5d;
    else if(k>=0.455d && k<0.708d    )Pe=0.4d;
    else if(k>=0.708d && k<1.323d    )Pe=0.25d;
    else if(k>=1.323d && k<2.072d    )Pe=0.15d;
    else if(k>=2.072d && k<2.706d    )Pe=0.10d;
    else if(k>=2.706d && k<3.841d    )Pe=0.05d;
    else if(k>=3.841d && k<5.024d    )Pe=0.025d;
    else if(k>=5.024d && k<6.635d    )Pe=0.01d;
    else if(k>=6.635d && k<7.879d    )Pe=0.005d;
    else if(k>=7.879d && k<10.828d    )Pe=0.001d;
    else if(k>=10.828d        )Pe=0.001d;
    lua_pushnumber(L,k);
    lua_pushnumber(L,Pe);
    return 2;
  }
  void luaopen(lua_State * L){
    luaL_Reg reg[]={
      "num",sta_num,
      "independence",sta_independence,
      "regression",sta_regression,
      NULL,NULL
    };
    lua_newtable(L);
    luaL_setfuncs(L, reg,0);
    lua_setglobal(L,"Sta");
  }
 }
}
#endif