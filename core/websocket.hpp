#ifndef yrssf_core_websocket
#define yrssf_core_websocket
#include "httpd.hpp"
#include "global.hpp"
#include "base64.hpp"
#include "rwmutex.hpp"
#include <string>
#include <list>
#include <openssl/sha.h>
namespace yrssf{
  namespace websocket{
    typedef enum{
      WS_OPENING_FRAME  =0xF3,
      WS_ERROR_FRAME    =0xF1,
      WS_TEXT_FRAME     =0x01,
      WS_BINARY_FRAME   =0x02,
      WS_PING_FRAME     =0x09,
      WS_PONG_FRAME     =0x0A,
      WS_CLOSING_FRAME  =0x08,
      WS_EMPTY_FRAME    =0xF0
    }WS_FrameType;
    class Websocket{
      public:
      std::list<int> fds;
      RWMutex locker;
      Websocket(){
      }
      ~Websocket(){
        locker.Wlock();
        for(auto fd:fds){
          close(fd);
        }
        locker.unlock();
      }
      void add(int fd){
        locker.Wlock();
        fds.push_back(fd);
        locker.unlock();
      }
    }wsock;
    int encodeFrame(std::string,std::string &,WS_FrameType);
    int wsSend(int fd,const std::string & data){
      std::string res;
      encodeFrame(data,res,WS_TEXT_FRAME);
      return send(fd,res.c_str(),res.length(),0);
    }
    int boardcast(const std::string & data){
      wsock.locker.Rlock();
      for(auto itb=wsock.fds.begin();itb!=wsock.fds.end();){
        auto it=itb++;
        int fd=*it;
        if(wsSend(fd, data)==-1){
          close(fd);
          wsock.fds.erase(it);
        }
      }
      wsock.locker.unlock();
    }
    std::string sha1(const std::string & in){
      char buf[128];
      SHA1(
        (const unsigned char*)in.c_str(),
        in.length(),
        (unsigned char*)buf
      );
      return std::string(buf);
    }
    int decodeFrame(std::string inFrame,std::string &outMessage){
      int ret = WS_OPENING_FRAME;
      const char *frameData = inFrame.c_str();
      const int frameLength = inFrame.size();
      if (frameLength < 2){
        ret = WS_ERROR_FRAME;
      }
      // 检查扩展位并忽略
      if ((frameData[0] & 0x70) != 0x0){
        ret = WS_ERROR_FRAME;
      }
      // fin位: 为1表示已接收完整报文, 为0表示继续监听后续报文
      ret = (frameData[0] & 0x80);
      if ((frameData[0] & 0x80) != 0x80){
        ret = WS_ERROR_FRAME;
      }
      // mask位, 为1表示数据被加密
      if ((frameData[1] & 0x80) != 0x80){
        ret = WS_ERROR_FRAME;
      }
      // 操作码
      uint16_t payloadLength = 0;
      uint8_t payloadFieldExtraBytes = 0;
      uint8_t opcode = static_cast<uint8_t >(frameData[0] & 0x0f);
      if (opcode == WS_TEXT_FRAME){
        // 处理utf-8编码的文本帧
        payloadLength = static_cast<uint16_t >(frameData[1] & 0x7f);
        if (payloadLength == 0x7e){
            uint16_t payloadLength16b = 0;
            payloadFieldExtraBytes = 2;
            memcpy(&payloadLength16b, &frameData[2], payloadFieldExtraBytes);
            payloadLength = ntohs(payloadLength16b);
        }else if (payloadLength == 0x7f){
            // 数据过长,暂不支持
            ret = WS_ERROR_FRAME;
        }
    }else if (opcode == WS_BINARY_FRAME || opcode == WS_PING_FRAME || opcode == WS_PONG_FRAME){
        // 二进制/ping/pong帧暂不处理
    }else if (opcode == WS_CLOSING_FRAME){
        ret = WS_CLOSING_FRAME;
    }else{
        ret = WS_ERROR_FRAME;
    }
    // 数据解码
    if ((ret != WS_ERROR_FRAME) && (payloadLength > 0)){
        // header: 2字节, masking key: 4字节
        const char *maskingKey = &frameData[2 + payloadFieldExtraBytes];
        char *payloadData = new char[payloadLength + 1];
        memset(payloadData, 0, payloadLength + 1);
        memcpy(payloadData, &frameData[2 + payloadFieldExtraBytes + 4], payloadLength);
        for (int i = 0; i < payloadLength; i++){
            payloadData[i] = payloadData[i] ^ maskingKey[i % 4];
        }
        outMessage = payloadData;
        delete[] payloadData;
      }
      return ret;
    }
    int encodeFrame(std::string inMessage, std::string &outFrame, WS_FrameType frameType){
      int ret = WS_EMPTY_FRAME;
      const uint32_t messageLength = inMessage.size();
      if (messageLength > 32767){
        // 暂不支持这么长的数据
        return WS_ERROR_FRAME;
      }
      uint8_t payloadFieldExtraBytes = (messageLength <= 0x7d) ? 0 : 2;
      // header: 2字节, mask位设置为0(不加密), 则后面的masking key无须填写, 省略4字节
      uint8_t frameHeaderSize = 2 + payloadFieldExtraBytes;
      uint8_t *frameHeader = new uint8_t[frameHeaderSize];
      memset(frameHeader, 0, frameHeaderSize);
      // fin位为1, 扩展位为0, 操作位为frameType
      frameHeader[0] = static_cast<uint8_t>(0x80 | frameType);
      // 填充数据长度
      if (messageLength <= 0x7d){
        frameHeader[1] = static_cast<uint8_t>(messageLength);
      }else{
        frameHeader[1] = 0x7e;
        uint16_t len = htons(messageLength);
        memcpy(&frameHeader[2], &len, payloadFieldExtraBytes);
      }
      // 填充数据
      uint32_t frameSize = frameHeaderSize + messageLength;
      char *frame = new char[frameSize + 1];
      memcpy(frame, frameHeader, frameHeaderSize);
      memcpy(frame + frameHeaderSize, inMessage.c_str(), messageLength);
      frame[frameSize] = '\0';
      outFrame = frame;
      delete[] frame;
      delete[] frameHeader;
      return ret;
    }
    bool(*onWebsocketConnect)(httpd::request *)=[](httpd::request *){
      return true;
    };
    void callback(httpd::request * req){
      if(onWebsocketConnect==NULL)
        close(req->fd);
      
      req->init();
      req->query_decode();
      req->getcookie();
      req->cookie_decode();
      
      if(
        req->paseredheader["upgrade"]!="websocket" ||
        req->paseredheader["connection"]!="Upgrade"
      ){
        close(req->fd);
        return;
      }
      
      auto it=req->paseredheader.find("sec-websocket-key");
      if(it==req->paseredheader.end()){
        close(req->fd);
        return;
      }
      
      static const std::string magicKey("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
      std::string serverKey = (it->second) + magicKey;
      std::string shahash=sha1(serverKey);
      
      char bufr[512];
      base64::encode(
        (const unsigned char*)shahash.c_str(),
        shahash.length(),
        (unsigned char*)bufr
      );
      
      httpd::writeStr(req->fd,
        "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: upgrade\r\n"
        "Sec-WebSocket-Accept: "
      );
      
      httpd::writeStr(req->fd,bufr);
      
      httpd::writeStr(req->fd,"\r\n\r\n");
      
      if(!onWebsocketConnect(req))
        close(req->fd);
      else
        wsock.add(req->fd);
    }
  }
}
#endif