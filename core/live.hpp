#ifndef yrssf_core_live
#define yrssf_core_live
#include "client.hpp"
#include "websocket.hpp"
#include <sys/file.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <jpeglib.h>
#include <jerror.h>
#include <sys/soundcard.h>
#include <string>
namespace yrssf{
int livefifo;
int liveserverfifo;
bool runliveclient_cb(void * data,int size,void*);
static void * runliveclient_th(void *){
  if(clientdisabled)return 0;
  clientdisabled=1;
  clientlocker.lock();
  client.liveclientrunning=1;
  ysDebug("live mode on");
  client.liveclient(runliveclient_cb,NULL);
  ysDebug("live mode off");
  clientlocker.unlock();
  clientdisabled=0;
}
static int runliveclient(){
  pthread_t newthread;
  if(pthread_create(&newthread,NULL,runliveclient_th,NULL)!=0)
    perror("pthread_create");
}
static void * liveserver_cb(void * arg){
  auto req=(require*)arg;
  auto bufm=(netSource*)(req->buffer);
  client.live(bufm);
  //ysDebug("live size=%d",bufm->size());
  pool.del(req);
}
require *  liveserverreq;
static void * liveserver(void *){
  int lfd,len;
  pthread_t newthread;
  liveserverreq=pool.get();
  auto bufm=(netSource*)(liveserverreq->buffer);
  auto buf=bufm->source;
  while(1){
    lfd=open("live/server",O_RDONLY);
    ysDebug("live begin");
    while(len=read(lfd,buf,SOURCE_CHUNK_SIZE)){
      bufm->size=len;
      if(pthread_create(&newthread,NULL,liveserver_cb,liveserverreq)!=0){
        //perror("pthread_create");
        liveserver_cb(liveserverreq);
      }
      liveserverreq=pool.get();
      bufm=(netSource*)(liveserverreq->buffer);
      buf=bufm->source;
    }
    ysDebug("live end");
    close(lfd);
  }
}
namespace videolive{
  class Sound{
    int fd;
    public:
    char buffer[3584];
    Sound(const char * path){
      fd = open(path, O_RDWR);
      if(fd < 0){
      printf("can not open dev\n");
        return;
      }
      int i;
      //设置参数
      i=0;
      ioctl (fd,SNDCTL_DSP_STEREO,&i);                //单声道
      ioctl (fd,SNDCTL_DSP_RESET,(char *)&i) ;
      ioctl (fd,SNDCTL_DSP_SYNC,(char *)&i);
      i=1;
      ioctl (fd,SNDCTL_DSP_NONBLOCK,(char *)&i);
      i=8000;
      ioctl (fd,SNDCTL_DSP_SPEED,(char *)&i);         //频率
      i=1;
      ioctl (fd,SNDCTL_DSP_CHANNELS,(char *)&i);
      i=8;
      ioctl (fd,SNDCTL_DSP_SETFMT,(char *)&i);
      i=3;
      ioctl (fd,SNDCTL_DSP_SETTRIGGER,(char *)&i);
      i=3;
      ioctl (fd,SNDCTL_DSP_SETFRAGMENT,(char *)&i);
      i=1;
      ioctl (fd,SNDCTL_DSP_PROFILE,(char *)&i);
    }
    ~Sound(){
      if(fd>=0)close(fd);
    }
    void readbuffer(){
      if(fd){
        bzero(buffer,3584);
        read (fd,buffer,3584);
      }
    }
    void readbuffer(void * buf){
      if(fd){
        bzero(buffer,3584);
        read (fd,buf,3584);
      }
    }
    void display(){
      if(fd) write(fd,buffer,3584);
    }
    void display(void * data){
      if(fd) write(fd,data,3584);
    }
  };
  struct rgb565{
    unsigned short int pix565;
    void torgb(int * r,int * g,int * b){
      *r = ((pix565)>>11)&0x1f;
      *g = ((pix565)>>5)&0x3f;
      *b = (pix565)&0x1f;
    }
  };
  class shot{
    int fd;
    public:
    rgb565 * data;
    struct fb_var_screeninfo fb_var_info;
    struct fb_fix_screeninfo fb_fix_info;
    int buffer_size;
    shot(const char * path){
      fd = open(path, O_RDONLY);
      if(fd < 0){
      printf("can not open dev\n");
        return;
      }
      // 获取LCD的可变参数
      ioctl(fd, FBIOGET_VSCREENINFO, &fb_var_info);
      // 一个像素多少位    
      printf("bits_per_pixel: %d\n", fb_var_info.bits_per_pixel);
      // x分辨率
      printf("xres: %d\n", fb_var_info.xres);
      // y分辨率
      printf("yres: %d\n", fb_var_info.yres);
      // r分量长度(bit)
      printf("red_length: %d\n", fb_var_info.red.length);
      // g分量长度(bit)
      printf("green_length: %d\n", fb_var_info.green.length);
      // b分量长度(bit)
      printf("blue_length: %d\n", fb_var_info.blue.length);
      // t(透明度)分量长度(bit)
      printf("transp_length: %d\n", fb_var_info.transp.length);
      // r分量偏移
      printf("red_offset: %d\n", fb_var_info.red.offset);
      // g分量偏移
      printf("green_offset: %d\n", fb_var_info.green.offset);
      // b分量偏移
      printf("blue_offset: %d\n", fb_var_info.blue.offset);
      // t分量偏移
      printf("transp_offset: %d\n", fb_var_info.transp.offset);

      // 获取LCD的固定参数
      ioctl(fd, FBIOGET_FSCREENINFO, &fb_fix_info);
      // 一帧大小
      printf("smem_len: %d\n", fb_fix_info.smem_len);
      // 一行大小
      printf("line_length: %d\n", fb_fix_info.line_length);
      // 一帧大小
      buffer_size = (fb_var_info.xres * fb_var_info.yres * fb_var_info.bits_per_pixel / 8);
      data = (rgb565 *)malloc(buffer_size);
    }
    ~shot(){
      if(fd>=0)close(fd);
      free(data);
    }
    int readbuffer(){
      return read(fd, data, buffer_size);
    }
    rgb565 * getpix(int x,int y){
      if(x<0 || y<0 || x>fb_var_info.xres || y>fb_var_info.yres) return NULL;
      return &data[x+y*fb_var_info.xres];
    }
    int RGB565_to_RGB24(void * rgb565, unsigned char *rgb24, int width, int height){
      int i;
      int whole = width*height;
      unsigned char r, g, b;
      unsigned short int *pix565;
      pix565 = (unsigned short int *)rgb565;
      for(i = 0;i < whole;i++){
        r = ((*pix565)>>11)&0x1f;
        *rgb24 = (r<<3) | (r>>2);
        rgb24++;
        g = ((*pix565)>>5)&0x3f;
        *rgb24 = (g<<2) | (g>>4);
        rgb24++;
        b = (*pix565)&0x1f;
        *rgb24 = (b<<3) | (b>>2);
        rgb24++;
        pix565++;
      }
      return 1;
    }
    int jpeg_compress(unsigned char *rgb, int width, int height,const char * path){
      struct jpeg_compress_struct cinfo;
      struct jpeg_error_mgr jerr;
      FILE * outfile;
      JSAMPROW row_pointer[1];
      int row_stride;
      cinfo.err = jpeg_std_error(&jerr);
      jpeg_create_compress(&cinfo);
      if ((outfile = fopen(path, "wb")) == NULL){
        printf("can not open out.jpg\n");
        return -1;
      }
      jpeg_stdio_dest(&cinfo, outfile);
      cinfo.image_width = width;
      cinfo.image_height = height;
      cinfo.input_components = 3;// 1-灰度图，3-彩色图
      // 输入数据格式为RGB
      cinfo.in_color_space = JCS_RGB;// JCS_GRAYSCALE-灰度图，JCS_RGB-彩色图
      jpeg_set_defaults(&cinfo);
      jpeg_set_quality(&cinfo, 80, TRUE);// 设置压缩质量：80
      jpeg_start_compress(&cinfo, TRUE);// 开始压缩过程
      row_stride = width * 3;// row_stride: 每一行的字节数
      while (cinfo.next_scanline < cinfo.image_height){
        row_pointer[0] = &rgb[cinfo.next_scanline * row_stride];
        (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
      }
      jpeg_finish_compress(&cinfo);// 完成压缩过程
      fclose(outfile);
      jpeg_destroy_compress(&cinfo);// 释放资源
      return 1;
    }
    void save(const char * path){
      auto rgb = (unsigned char *)malloc(fb_var_info.xres * fb_var_info.yres * 3);
      readbuffer();
      RGB565_to_RGB24(data, rgb, fb_var_info.xres, fb_var_info.yres);
      // jpeg压缩
      if(jpeg_compress(rgb, fb_var_info.xres, fb_var_info.yres,path) < 0)
        printf("compress failed!\n");
      free(rgb);
    }
  };
  struct pixel{
    uint8_t R,G,B;
    pixel(){}
    pixel(uint8_t r,uint8_t g,uint8_t b){
      R=r;
      G=g;
      B=b;
    }
  };
  struct netPack{
    char m;
    nint32 x,y,w,h,t;
    pixel data[];
  };
  class boardcast{
    public:
    shot  shotbuf;
    float resizex,resizey;
    pixel buffer[600][500];
    struct Updatelog{
      unsigned int times,ct;
      Updatelog(){
        this->times=0;
      }
      bool needsend(){
        double u=(double)ct;
        double d=300.0d;
        if((u/d)>config::livepresent){
          return 1;
        }else{
          return 0;
        }
      }
      void count(){
        ct++;
      }
    }updatelog[40][25];
    unsigned int times;
    //chunk:
    //width=15     //15*40
    //height=20    //20*25
    boardcast(const char * path):shotbuf(path),resizex(1),resizey(1){
      resizex=shotbuf.fb_var_info.xres/600.0f;
      resizey=shotbuf.fb_var_info.yres/500.0f;
      times=0;
    }
    ~boardcast(){
    }
    void setpixel(pixel * px,int x,int y){
      if(x<0 || y<0 || x>=600 || y>=500) return;
      int cx=x/15;
      int cy=y/20;
      if(
        buffer[x][y].R!=px->R ||
        buffer[x][y].G!=px->G ||
        buffer[x][y].B!=px->B
      ){
        if(updatelog[x][y].times!=this->times){
          updatelog[x][y].times=this->times;
          updatelog[x][y].ct=0;  //重置计数器
        }
        updatelog[x][y].count();
      }
      buffer[x][y].R=px->R;
      buffer[x][y].G=px->G;
      buffer[x][y].B=px->B;
    }
    void sendall(){
      int ix,iy;
      netSource nbuf;
      char * buf=nbuf.source;
      nbuf.size=SOURCE_CHUNK_SIZE;
      void * endp=&(buf[SOURCE_CHUNK_SIZE]);
      netPack * bufp=(netPack*)buf;
      pixel * pxl;
      //printf("send a chunk\n");
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
        bufp->m='w';
        client.live(&nbuf);
      }
    }
    void sendshot(){
      shotbuf.readbuffer();
      rgb565 * pix;
      int r,g,b;
      //printf("send a frame\n");
      for(int ix=0;ix<600;ix++)
      for(int iy=0;iy<500;iy++){
        pix=shotbuf.getpix(ix*resizex,iy*resizey);
        if(pix==NULL){
          pix->torgb(&r,&g,&b);
          buffer[ix][iy].R=r/256;
          buffer[ix][iy].G=g/256;
          buffer[ix][iy].B=b/256;
        }
      }
      sendall();
    }
    void liveshot(){
      int ix,iy,jx,jy,j;
      rgb565 * pix;
      pixel px;
      netSource nbuf;
      char * buf=nbuf.source;
      nbuf.size=SOURCE_CHUNK_SIZE;
      void * endp=&(buf[SOURCE_CHUNK_SIZE]);
      netPack * bufp=(netPack*)buf;
      for(ix=0;ix<600;ix++)
      for(iy=0;iy<500;iy++){
        pix=shotbuf.getpix(ix*resizex,iy*resizey);
        if(pix==NULL){
          int r,g,b;
          pix->torgb(&r,&g,&b);
          px.R=r/256;
          px.G=g/256;
          px.B=b/256;
          setpixel(&px,ix,iy);
        }
      }
      for(ix=0;ix<40;ix++)
      for(iy=0;iy<25;iy++){
        if(updatelog[ix][iy].times==times && updatelog[ix][iy].needsend()){
          bufp->m='i';
          bufp->x=ix;
          bufp->y=iy;
          bufp->t=times;
          j=0;
          for(jy=0;jy<20;jy++)
          for(jx=0;jx<15;jx++){
            bufp->data[j]=pixel(
              buffer[jx][jy].R,
              buffer[jx][jy].G,
              buffer[jx][jy].B
            );
            j++;
          }
          client.live(&nbuf);
        }else
          continue;
      }
      times++;
    }
  };
  class sound{
    public:
    unsigned int times;
    Sound soundbuf;
    sound(const char * path):soundbuf(path){
      times=0;
    }
    void sendsound(){
      times++;
      netSource nbuf;
      int i;
      char * buf=nbuf.source;
      nbuf.size=SOURCE_CHUNK_SIZE;
      void * endp=&(buf[SOURCE_CHUNK_SIZE]);
      netPack * bufp=(netPack*)buf;
      bufp->m='s';
      bufp->t=times;
      char * sp=(char*)bufp->data;
      soundbuf.readbuffer(sp);
      /*
      for(i=0;i<3584;i++){
        *sp=soundbuf.buffer[i];
        sp++;
      }
      */
      client.live(&nbuf);
    }
  };
  class live_f{
    public:
    boardcast * bd;
    sound     * sd;
    live_f(){
      bd=NULL;
    }
    ~live_f(){
      if(bd)delete bd;
    }
    void init(const char * path){
      if(bd!=NULL) return;
      bd=new boardcast(path);
    }
    void initSound(const char * path){
      if(sd!=NULL) return;
      sd=new sound(path);
    }
    void destory(){
      if(bd)delete bd;
      bd=NULL;
    }
    void destorySound(){
      if(sd)delete sd;
      sd=NULL;
    }
    void sendall(){
      if(!bd)return;
      bd->sendall();
    }
    void liveall(){
      if(!bd)return;
      bd->liveshot();
    }
    void save(const char * path){
      if(!bd)return;
      bd->shotbuf.save(path);
    }
    void sendsound(){
      if(!bd)return;
      sd->sendsound();
    }
  }screen;
}
int livetimer=0;
bool runliveclient_cb(void * data,int size,void*){
  auto np=(videolive::netPack*)data;
  char buf[512];
  std::string sbuf;
  if(np->m=='i' || np->m=='w'){
    if(np->t() > livetimer){
      livetimer==np->t();
      snprintf(buf,512,"live %c %8x %8x %8x %8x : ",
        np->m,
        np->x(),
        np->y(),
        np->w(),
        np->h()
      );
      sbuf=buf;
      int j=0;
      for(int jy=0;jy<20;jy++){
        for(int jx=0;jx<15;jx++){
          bzero(buf,64);
          snprintf(buf,64,"%2x,%2x,%2x;",
            np->data[j].R,
            np->data[j].G,
            np->data[j].B
          );
          sbuf+=buf;
          j++;
        }
      }
      websocket::boardcast(sbuf);
    }
  }else
  if(np->m=='s'){
    auto bufp=(unsigned char *)np->data;
    sbuf="live s : ";
    unsigned char bsound[3584*2];
    base64::encode(
      bufp,3584,bsound
    );
    sbuf+=(char*)bsound;
    websocket::boardcast(sbuf);
    if(config::soundputout){
      if(videolive::screen.sd){
        videolive::screen.sd->soundbuf.display(bufp);
      }
    }
  }
  if(config::liveputout){
    write(livefifo,data,size>SOURCE_CHUNK_SIZE?SOURCE_CHUNK_SIZE:size);
  }
  return 1;
}
//////////////////////////////////////
}
#endif