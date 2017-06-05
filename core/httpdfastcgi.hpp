#ifndef yrssf_httpd_fastcgi
#define yrssf_httpd_fastcgi

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "global.hpp"

#define      MAX_BUF_SIZE    8096
#define      MAX_RECV_SIZE   4000

#define FCGI_VERSION_1      1
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
    unsigned short FCGI_PORT=9000;
    char  FCGI_HOST[32]="127.0.0.1";
    void setport(unsigned short port){
      FCGI_PORT=port;
    }
    void sethost(const char * addr){
      if(strlen(addr)>=31)return;
      strcpy(FCGI_HOST,addr);
    }
    struct FCGI_Header{
      unsigned char version;
      unsigned char type;
      unsigned char requestIdB1;
      unsigned char requestIdB0;
      unsigned char contentLengthB1;
      unsigned char contentLengthB0;
      unsigned char paddingLength;
      unsigned char reserved;
      void init(int type, int requestId, int contentLength, int paddingLength){
        version          = FCGI_VERSION_1;
        type             = (unsigned char)type;        
        requestIdB1      = (unsigned char) ((requestId     >> 8) & 0xff);
        requestIdB0      = (unsigned char) ((requestId         ) & 0xff);
        contentLengthB1  = (unsigned char) ((contentLength >> 8) & 0xff);
        contentLengthB0  = (unsigned char) ((contentLength     ) & 0xff);
        paddingLength    = (unsigned char)paddingLength;
        reserved         = 0;
      }
    };
    struct FCGI_BeginRequestBody{
      unsigned char roleB1;
      unsigned char roleB0;
      unsigned char flags;
      unsigned char reserved[5];
      void init(int role){
        roleB1 = (unsigned char) ((role >>  8) & 0xff);
        roleB0 = (unsigned char) (role         & 0xff);
        flags  = (unsigned char) 0;        //关闭线路
        memset(reserved, 0, sizeof(reserved));
      }
    };
    struct FCGI_BeginRequestRecord{
      FCGI_Header header;
      FCGI_BeginRequestBody body;
    };
    struct FCGI_ParamsRecord{
      FCGI_Header header;
      unsigned char nameLength;
      unsigned char valueLength;
      unsigned char data[0];
    };
    struct FCGI_EndRequestBody{
      unsigned char appStatusB3;
      unsigned char appStatusB2;
      unsigned char appStatusB1;
      unsigned char appStatusB0;
      unsigned char protocolStatus;   // 协议级别的状态码
      unsigned char reserved[3];
    };
    struct FCGI_EndRequestRecord{
      FCGI_Header header;
      FCGI_EndRequestBody body;
    };
    struct http_header{
      char method[10];           //请求方式
      char path[256];            //文件路径
      //char filename[256];      //请求文件名
      //char version[10];        //HTTP协议版本
      //char url[256];           //请求url
      char param[256];           //请求参数
      char contype[256];         //消息类型
      char conlength[16];        //消息长度
      //char ext[10];            //文件后缀
      //char *content;             //body
    };
    
    FCGI_Header makeHeader(int type, int requestId, int contentLength, int paddingLength){
      FCGI_Header fastcgi_header;
      fastcgi_header.version = FCGI_VERSION_1;
      fastcgi_header.type = (unsigned char)type;      
      fastcgi_header.requestIdB1      = (unsigned char) ((requestId     >> 8) & 0xff);
      fastcgi_header.requestIdB0      = (unsigned char) ((requestId         ) & 0xff);
      fastcgi_header.contentLengthB1  = (unsigned char) ((contentLength >> 8) & 0xff);
      fastcgi_header.contentLengthB0  = (unsigned char) ((contentLength     ) & 0xff);
      fastcgi_header.paddingLength = (unsigned char)paddingLength;
      fastcgi_header.reserved = 0;
      return fastcgi_header;
    }
    FCGI_BeginRequestBody makeBeginRequestBody( int role){
      FCGI_BeginRequestBody fastcgi_body;
      fastcgi_body.roleB1 = (unsigned char) ((role >>  8) & 0xff);
      fastcgi_body.roleB0 = (unsigned char) (role         & 0xff);
      fastcgi_body.flags  = (unsigned char) 0;      //关闭线路
      memset(fastcgi_body.reserved, 0, sizeof(fastcgi_body.reserved));
      return fastcgi_body;
    }
    
    int buffer_path_simplify(char *dest, char *src){  
      int count;  
      char c;  
      char *start, *slash, *walk, *out;  
      char pre; //当前匹配的前一个字符  
                                
      if (src == NULL ||  dest == NULL)  
            return -1;  
                                      
      walk  = src;  
      start = dest;  
      out   = dest;  
      slash = dest;  
                                                              
      while (*walk == ' ') {  
            walk++;  
      }  
                                                                    
      pre = *(walk++);  
      c    = *(walk++);  
      if (pre != '/') {  
            *(out++) = '/';  
      }  
      *(out++) = pre;  
                                                                                            
      if (pre == '\0') {  
            *out = 0;  
            return 0;  
      } 
      
      while (1) {  
            if (c == '/' || c == '\0') {  
                  count = out - slash;  
                  if (count == 3 && pre == '.') {  
                        out = slash;  
                        if (out > start) {  
                              out--;  
                              while (out > start && *out != '/') {  
                                    out--;  
                              }  
                        }  
                        if (c == '\0')  
                              out++;  
                  } else if (count == 1 || pre == '.') {  

                        out = slash;  
                        if (c == '\0')  
                              out++;  
                  }  
                  slash = out;  
            }  

            if (c == '\0')  
                  break;   
            pre = c;  
            c    = *walk;  
            *out = pre;  
            out++;  
            walk++;  
      }  
       *out = '\0';  
       return 0;  
    }
    
    void send_client(char *ok, int outlen, char *err, int errlen, int client_fd, http_header *hr){
      char header[MAX_BUF_SIZE], header_buf[256];
      char *body, *start, *end, mime[256];
      int header_len, n;

      sprintf(header, "HTTP/1.0 200 OK\r\n");
      body = strstr(ok, "\r\n\r\n") + 4;

      header_len = (int)(body - ok);      //头长度
      strncpy(header_buf, ok, header_len);
      sprintf(header, "%sContent-Length: %d\r\n", header,errlen + outlen - header_len);
                  
      /*提取mime */
      start = strstr(header_buf, "Content-type");
      if(start != NULL){
            start = index(start, ':') + 2;
            end = index(start, '\r');
            n = end - start;
            strncpy(mime, start, n);
      }else{
            strcpy(mime, "text/html");
      }
      sprintf(header, "%sContent-Type: %s\r\n\r\n", header,mime);
      send(client_fd, header, strlen(header), 0);
      send(client_fd, body, outlen - header_len, 0);
      // debug printf("收到的stdin:\n%s\n收到的stderr:\n%s\n", ok, err);
      if(errlen > 0){
            send(client_fd, err, errlen, 0);
      }
    }

    int conn_fastcgi(){      
      int fcgi_fd;
      struct sockaddr_in fcgi_sock;

      memset(&fcgi_sock, 0, sizeof(fcgi_sock));
      fcgi_sock.sin_family = AF_INET;
      fcgi_sock.sin_addr.s_addr = inet_addr(FCGI_HOST);
      fcgi_sock.sin_port = htons(FCGI_PORT);
      
      if(-1 == (fcgi_fd = socket(PF_INET, SOCK_STREAM, 0))){
            ysError("fcgi socket()");
            return -1;
      }
      
      if(-1 == connect(fcgi_fd, (struct sockaddr *)&fcgi_sock, sizeof(fcgi_sock))){
            ysError("php-fpm connect");
            return -1;
      }
      return fcgi_fd;
    }
    
    int send_fastcgi(int fcgi_fd, int client_fd, http_header * hr){
            
      char filename[50];
      int requestId = client_fd, recvbytes, sendbytes;

      /* 发送FCGI_BEGIN_REQUEST */      
      FCGI_BeginRequestRecord beginRecord;
      beginRecord.body = makeBeginRequestBody(FCGI_RESPONDER);
      beginRecord.header =  makeHeader(FCGI_BEGIN_REQUEST, requestId, sizeof(beginRecord.body), 0);
      if(-1 == (sendbytes = send(fcgi_fd, &beginRecord, sizeof(beginRecord), 0))){
            ysError("fcgi send beginRecord error");
            return -1;
      }


      getcwd(filename, sizeof(filename));
      strcat(filename, "/");
      strcat(filename, hr->path);
      buffer_path_simplify(filename, filename);
      const char *params[][2] = {{"SCRIPT_FILENAME", filename}, {"REQUEST_METHOD", hr->method}, {"QUERY_STRING", hr->param}, {"CONTENT_TYPE", hr->contype },{"CONTENT_LENGTH", hr->conlength },{"", ""}};
      /* 发送 FCGI_PARAMS */
      int i, conLength, paddingLength;
      FCGI_ParamsRecord *paramsRecord;
      FCGI_Header emptyData;
      for(i = 0; params[i][0] != ""; i++){   // debug printf("%s : %s\n", params[i][0], params[i][1]);
            conLength = strlen(params[i][0]) + strlen(params[i][1]) + 2;
            paddingLength = (conLength % 8) == 0 ? 0 : 8 - (conLength % 8);
            paramsRecord = (FCGI_ParamsRecord *)malloc(sizeof(FCGI_ParamsRecord) + conLength + paddingLength);
            paramsRecord->nameLength = (unsigned char)strlen(params[i][0]);    // 填充参数值
            paramsRecord->valueLength = (unsigned char)strlen(params[i][1]);   // 填充参数名
            paramsRecord->header = makeHeader(FCGI_PARAMS, requestId, conLength, paddingLength);
            memset(paramsRecord->data, 0, conLength + paddingLength);
            memcpy(paramsRecord->data, params[i][0], strlen(params[i][0]));
            memcpy(paramsRecord->data + strlen(params[i][0]), params[i][1], strlen(params[i][1]));
            if(-1 == (sendbytes = send(fcgi_fd, paramsRecord, 8 + conLength + paddingLength, 0))){
                  ysError("fcgi send paramsRecord error");
                  return -1;
            }
            free(paramsRecord);
      }
      /* 空的FCGI_PARAMS参数 */
      emptyData = makeHeader(FCGI_PARAMS, requestId, 0, 0);
      if(-1 == (sendbytes = send(fcgi_fd, &emptyData, FCGI_HEADER_LEN, 0))){
            ysError("fcgi send paramsRecord empty error");
            return -1;
      }

      /*POST　并且有请求体 才会发送FCGI_STDIN数据 */
      char buf[8] = {0};
      char bufcon[8];
      int len = atoi(hr->conlength);
      int send_len;
      if(!strcmp("POST", hr->method)){
            // debug printf("post数据总长度:%d\n", len);
            // debug printf("%s\n", hr->content);
            while(len > 0){
                  send_len = len > FCGI_MAX_LEN  ? FCGI_MAX_LEN : len;
                  len -= send_len;
                  FCGI_Header stdinHeader;
                  paddingLength = (send_len %  8) == 0 ? 0 :  8 - (send_len % 8);
                  stdinHeader = makeHeader(FCGI_STDIN, requestId, send_len, paddingLength);
                  if(-1 == (sendbytes = send(fcgi_fd, &stdinHeader,FCGI_HEADER_LEN, 0))){
                        ysError("fcgi send stdinHeader error");
                        return -1;
                  }
                  
                  int lencon;
                  for(int j=0;j<send_len;j++){
                    lencon=recv(client_fd,bufcon,sizeof(bufcon), 0);
                    if(lencon>0)
                      send(fcgi_fd, bufcon, lencon, 0);
                    else
                      break;
                  }
                                    
                  //if(-1 == (sendbytes = send(fcgi_fd, hr->content, send_len, 0))){
                  //      ysError("fcgi send stdin");
                  //      return -1;
                  //}
                  
                  // debug printf("单次发送大小:%d\n发送内容:\n%s\n", sendbytes, hr->content);
                  if((paddingLength > 0) && (-1 == (sendbytes = send(fcgi_fd, buf, paddingLength, 0)))){
                        ysError("fcgi send stdin padding");
                        return -1;
                  }
                  // debug printf("长度%d\n", (int)strlen(hr->content));
            }
      }

      //debug  printf("%d\n", sendbytes);
      /* 发送空的FCGI_STDIN数据 */
      FCGI_Header emptyStdin = makeHeader(FCGI_STDIN, requestId, 0, 0);
      if(-1 == (sendbytes = send(fcgi_fd, &emptyStdin, FCGI_HEADER_LEN, 0))){
            ysError("fcgi send stdinHeader enpty");
            return -1;
      }
      return 1;
    }
    
    void recv_fastcgi(int fcgi_fd, int client_fd, http_header *hr){

      FCGI_Header responseHeader;
      FCGI_EndRequestBody responseEnder;
      int recvbytes, recvId, ok_recved = 0, ok_recv = 0;      // ok_recved 已经从缓冲区读取的字节数目,  ok_recv 本次要从缓冲区读取的字节数
      int err_recved = 0, err_recv = 0;                       //同上
      char buf[8];
      char header[MAX_BUF_SIZE];
      int contentLength;
      int errlen = 0, outlen = 0;
      char *ok = NULL , *err = NULL;

      /*接收数据 */
      while(recv(fcgi_fd, &responseHeader, FCGI_HEADER_LEN, 0) > 0){
            recvId = (int)(responseHeader.requestIdB1 << 8) + (int)(responseHeader.requestIdB0);
            if(FCGI_STDOUT == responseHeader.type && recvId == client_fd){
                  contentLength = ((int)responseHeader.contentLengthB1 << 8) + (int)responseHeader.contentLengthB0;
                  outlen += contentLength;
                  if(ok != NULL){
                        if(NULL == (ok =(char*) realloc(ok, outlen))){
                              ysError("realloc memory ok fail");
                              free(ok);
                              return ;
                        }
                  }else{
                        if(NULL == (ok = (char *)malloc(contentLength))){
                              ysError("alloc memory ok fail");
                              return ;
                        }
                  }
                  while(contentLength > 0){
                        //本次从缓冲区读取大小
                        ok_recv = contentLength > MAX_RECV_SIZE ? MAX_RECV_SIZE : contentLength;
                        if( -1 == (recvbytes = recv(fcgi_fd, ok + ok_recved, ok_recv, 0))){
                              ysError("fcgi recv stdout fail");
                              return ;
                        }
                        contentLength -= recvbytes;
                        ok_recved += recvbytes;
                  }      

                  if(responseHeader.paddingLength > 0){
                        recvbytes = recv(fcgi_fd, buf, responseHeader.paddingLength, 0);
                        if(-1 == recvbytes || recvbytes != responseHeader.paddingLength){
                              ysError("fcgi stdout padding fail");
                              return;
                        }
                  }
            }else if(FCGI_STDERR == responseHeader.type && recvId == client_fd){
                  contentLength = ((int)responseHeader.contentLengthB1 << 8) + (int)responseHeader.contentLengthB0;
                  errlen += contentLength;
                  if(err != NULL){
                        if( NULL == (err =(char*) realloc(err, errlen))){      
                              ysError("fcgi stderr realloc memory fail");
                              free(err);
                              return ;
                        }
                  }else{
                        if(NULL == (err = (char *)malloc(contentLength))){      
                              ysError("fcgi stderr alloc memory fail");
                              return ;
                        }
                  }
            
                  while(contentLength > 0){
                        //本次从缓冲区读取大小
                        err_recv = contentLength > MAX_RECV_SIZE ? MAX_RECV_SIZE : contentLength;
                        if( -1 == (recvbytes = recv(fcgi_fd, err + err_recved, err_recv, 0))){
                              ysError("fcgi recv stderr fail");
                              return ;
                        }
                        contentLength -= recvbytes;
                        err_recved += recvbytes;
                  }      

                  if(responseHeader.paddingLength > 0){
                        recvbytes = recv(fcgi_fd, buf, responseHeader.paddingLength, 0);
                        if(-1 == recvbytes || recvbytes != responseHeader.paddingLength){
                              ysError("fcgi stdout padding");
                              return ;
                        }
                  }
            }else if(FCGI_END_REQUEST == responseHeader.type && recvId == client_fd){
                  recvbytes = recv(fcgi_fd, &responseEnder, sizeof(responseEnder), 0);
                  
                  if(-1 == recvbytes || sizeof(responseEnder) != recvbytes){
                        free(err);
                        free(ok);
                        ysError("fcgi recv end fail");
                        return ;
                  }

                  /* 传输结束后构造http请求响应客户端 */
                  send_client(ok, outlen, err, errlen, client_fd, hr);
                  free(err);
                  free(ok);
            }
      }

    }
    
    void exec_fastcgi(int client_fd, http_header *hr){
      
      int fcgi_fd;
      
      /* 连接fastcgi服务器 */
      if(-1 == (fcgi_fd = conn_fastcgi())) return ;
      
      /* 发送数据 */
      if(-1 == (send_fastcgi(fcgi_fd, client_fd, hr))) return ;

      /* 接收数据 */
      recv_fastcgi(fcgi_fd, client_fd, hr);
      close(fcgi_fd);
    }
    
    int get_line(const int client_fd, char *buf, int size){
      char c = '\0';
      int n, i;
      for(i = 0; (c != '\n') &&  (i < size - 1); i++){
            n = recv(client_fd, &c, 1, 0);
            if(n > 0){
                  if('\r' == c){
                        n = recv(client_fd, &c, 1, MSG_PEEK);
                        if((n > 0) && ('\n' == c)){
                              recv(client_fd, &c, 1, 0);
                        }else{
                              c = '\n';
                        }
                  }
                  buf[i] = c;
            }else{
                  c = '\n';
            }
      }
      buf[i] = '\0';
      return i;
    }
    
    char *get_http_Val(char* str, const char *substr){
      char *start;
      if(strstr(str, substr) == NULL) return NULL;
      start = index(str, ':');
      //去掉空格冒号
      start += 2;
      //去掉换行符
      start[strlen(start) - 1] = '\0';
      return start;
    }
    void error_413(int connfd){
      char buf[1024];
      sprintf(buf, "HTTP/1.0 413 TOO LARGE\r\n");
      send(connfd, buf, strlen(buf), 0);
      sprintf(buf, "Server: yrssf-httpd/0.1.0\r\n");
      send(connfd, buf, strlen(buf), 0);
      sprintf(buf, "Content-Type: text/html\r\n");
      send(connfd, buf, strlen(buf), 0);
      sprintf(buf, "\r\n");
      send(connfd, buf, strlen(buf), 0);
      sprintf(buf, "<HTML><TITLE>TOO LARGE</TITLE>\r\n");
      send(connfd, buf, strlen(buf), 0);
      sprintf(buf, "<BODY>Too large.<br>\r\n");
      send(connfd, buf, strlen(buf), 0);
      sprintf(buf, "The body is too large.\r\n");
      send(connfd, buf, strlen(buf), 0);
      sprintf(buf, "</BODY></HTML>\r\n");
      send(connfd, buf, strlen(buf), 0);
    }
    void call(int connfd, const char *path, const char *method, const char *query_string){
      http_header hr;
      
      int i;
      int recvbytes;
      char *val;
      
      for(i=0;(i<9 && method[i]);i++)hr.method[i]=method[i];
      hr.method[i]='\0';
      
      //for(i=0;(i<255 && path[i]);i++)hr.path[i]=path[i];
      //hr.path[i]='\0';
      
      if(query_string && query_string[0]){
        for(i=0;(i<255 && query_string[i]);i++)hr.param[i]=query_string[i];
        hr.param[i]='\0';
      }else{
        hr.param[0]='\0';
      }
      
      char buf[512];
      //Content-Type以及Content-Length
      while((recvbytes > 0) && strcmp("\n", buf)){
            recvbytes = get_line(connfd, buf, MAX_BUF_SIZE);
            if((val = get_http_Val(buf, "Content-Length")) != NULL){
                  strcpy(hr.conlength, val);      
            }else if((val = get_http_Val(buf, "Content-Type")) != NULL){
                  strcpy(hr.contype, val);
            }
      }
      
      /* body内容  recv不能一次性接受很大值...  或者可以setsockopt 增大接受缓冲区*/
      int len = atoi(hr.conlength);
      if(len > config::httpBodyLength){
        error_413(connfd);
        return;
      }
      
      sprintf(hr.path, "%s%s","./static",path);
      
      exec_fastcgi(connfd,&hr);
    }
  }
}
#endif
