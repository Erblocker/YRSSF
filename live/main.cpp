#include <stdlib.h>
#include <SDL/SDL.h>
#include <netinet/in.h> 
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <sys/prctl.h>
#include <unistd.h>
#include "shot.hpp"
const char serverpipe[]="live/server";
const char clientpipe[]="live/client";
class nint32{
  public:
  int32_t value;
  nint32(){
    value=0;
  }
  nint32(int32_t v){
    value=htonl(v);
  }
  nint32 & operator=(int32_t v){
    value=htonl(v);
    return *this;
  }
  nint32(const nint32 & v){
    value=v.value;
  }
  nint32 & operator=(const nint32 & v){
    value=v.value;
    return *this;
  }
  int32_t val(){
    return ntohl(value);
  }
  int32_t operator()(){
    return val();
  }
};
class mywindow{
  public:
  SDL_Surface * surface;
  int width,height;
  mywindow(){
    width=600;
    height=500;
    if((SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_FULLSCREEN)==-1)) {
      exit(-1);
    }
    surface = SDL_SetVideoMode(width,height, 0, SDL_HWSURFACE | SDL_HWPALETTE | SDL_DOUBLEBUF );
    if( NULL == surface ) { 
      SDL_Quit();
      exit(-1);
    } 
  }
  ~mywindow(){
    SDL_FreeSurface(surface);//退出程序前必须释放 
    SDL_Quit();
  }
  struct pixel{
    Uint8 R,G,B;
    pixel(){}
    pixel(Uint8 r,Uint8 g,Uint8 b){
      R=r;
      G=g;
      B=b;
    }
  };
  void setpixel(pixel * px,int x,int y){
    if(x<0 || y<0 || x>=width || y>=height) return;
    Uint32* pixel = (Uint32*)surface->pixels;
    int ix=x+y*surface->pitch;
    pixel[ix]=SDL_MapRGB(surface->format,px->R,px->G,px->B);
  }
  void drawbitmap(pixel * px,int x,int y,int w,int h,void * end){
    SDL_LockSurface(surface);
    pixel p;
    int size=w*h;
    int ix,iy;
    pixel * ppx=px;
    for(iy=0;iy<h;iy++)
    for(ix=0;ix<w;ix++){
      if(ppx>end) return;
      setpixel(ppx,ix+x,iy+y);
      ppx++;
    }
    SDL_UnlockSurface(surface);
    SDL_UpdateRect(surface,x,y,w,h);
  }
}mw;
class connection{
  public:
  bool running;
  connection(){
    running=1;
  }
  ~connection(){}
  struct netPack{
    char m;
    nint32 x,y,w,h;
    mywindow::pixel data[];
  };
  void resolv(void * inp){
    netPack * in=(netPack*)inp;
    switch(in->m){
      case 'w':
        mw.drawbitmap(in->data,in->x(),in->y(),in->w(),in->h(),((char*)in)+4096);
      break;
    }
  }
  void autoresolv(){
    char buffer[4096];
    int fd=open(clientpipe,O_RDONLY);
    int len;
    SDL_Event myEvent;
    bzero(buffer,4096);
    while(running){
      SDL_PollEvent(&myEvent);
      if(myEvent.type==SDL_QUIT)break;
      len=read(fd,buffer,4096);
      if(len<=0)continue;
      resolv(buffer);
      bzero(buffer,len>4096?4096:len);
    }
    close(fd);
  }
}conn;
class boardcast{
  int fd;
  public:
  mywindow::pixel buffer[600][500];
  boardcast(){
    fd=open(serverpipe,O_WRONLY);
  }
  ~boardcast(){
    close(fd);
  }
  void setpixel(mywindow::pixel * px,int x,int y){
    if(x<0 || y<0 || x>=600 || y>=500) return;
    buffer[x][y].R=px->R;
    buffer[x][y].G=px->G;
    buffer[x][y].B=px->B;
  }
  void setarea(mywindow::pixel * px,int x,int y,int w,int h,void * end){
    int ix,iy;
    mywindow::pixel * ppx=px;
    for(iy=0;iy<h;iy++)
    for(ix=0;ix<w;ix++){
      if(ppx>end) return;
      setpixel(ppx,ix+x,iy+y);
      ppx++;
    }
    char buf[4096];
    void * endp=&buf[4096];
    connection::netPack * bufp=(connection::netPack*)buf;
    bufp->m='w';
    bufp->x=x;
    bufp->y=y;
    bufp->w=w;
    bufp->h=h;
    mywindow::pixel * pxl=&(bufp->data[0]);
    ppx=px;
    while(pxl<endp && ppx<end){
      pxl->R=ppx->R;
      pxl->G=ppx->G;
      pxl->B=ppx->B;
      pxl++;
      ppx++;
    }
    write(fd,bufp,4096);
  }
  void sendall(){
    int ix,iy;
    char buf[4096];
    void * endp=&(buf[4096]);
    connection::netPack * bufp=(connection::netPack*)buf;
    mywindow::pixel * pxl;
    for(iy=0;iy<(500/2);iy++){
      pxl=&(bufp->data[0]);
      bufp->x=0;
      bufp->y=iy*2;
      bufp->w=600;
      bufp->h=2;
      for(ix=0;ix<600;ix++){
        //line:iy*2
        pxl->R=buffer[ix][iy*2].R;
        pxl->G=buffer[ix][iy*2].G;
        pxl->B=buffer[ix][iy*2].B;
        pxl++;
      }
      for(ix=0;ix<600;ix++){
        //line:iy*2+1
        pxl->R=buffer[ix][iy*2+1].R;
        pxl->G=buffer[ix][iy*2+1].G;
        pxl->B=buffer[ix][iy*2+1].B;
        pxl++;
      }
      write(fd,bufp,4096);
    }
  }
};
int main(){
  system("killall -10 YRSSF");
  conn.autoresolv();
  system("killall -12 YRSSF");
  return 0;
}