#ifndef ys_live_shot
#define ys_live_shot
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <malloc.h>
#include <linux/fb.h>
namespace shot{
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
        exit(1);
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
  };
}
#endif