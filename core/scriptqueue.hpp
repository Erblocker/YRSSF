#ifndef yrssf_core_scriptqueue
#define yrssf_core_scriptqueue
#include "global.hpp"
#include <queue>
#include <string>
namespace yrssf{
  class Scriptqueue{
    std::queue<std::string> q;
    public:
    Scriptqueue(){}
    ~Scriptqueue(){}
    void insert(string & script){}
    void doscript(){}
  }scriptqueue;
}
#endif
