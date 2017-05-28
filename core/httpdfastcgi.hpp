#ifndef yrssf_httpd_fastcgi
#define yrssf_httpd_fastcgi

#include<stdio.h>
#include<string.h>
#include<dirent.h>
#include<pwd.h>
#include<unistd.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<ctype.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#define FCGI_VERSION_1      1
#define FCGI_PORT           9000
#define FCGI_HOST           "127.0.0.1"
#define FCGI_MAX_LEN        30000            //单次传输字节.  FASTCGI规定为0-65536
#define FCGI_HEADER_LEN     8
// 可用于type字段的值
#define FCGI_BEGIN_REQUEST  1
#define FCGI_ABORT_REQUEST  2
#define FCGI_END_REQUEST    3
#define FCGI_PARAMS         4
#define FCGI_STDIN          5
#define FCGI_STDOUT         6
#define FCGI_STDERR         7
#define FCGI_DATA           8
// 期望php-fpm扮演的角色
#define FCGI_RESPONDER      1
#define FCGI_AUTHORIZER     2
#define FCGI_FILTER         3

namespace yrssf{
  namespace fastcgi{
    void call(int connfd, const char *path, const char *method, const char *query_string){
      
    }
  }
}
#endif
