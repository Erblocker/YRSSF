#include <iostream>
#include <signal.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <pwd.h>
#include <pthread.h>
static const char * allowprocess[]={
  "com.sec.android.app.keyguard",
  "com.android.settings",
  "com.sec.android.widgetapp.alarmwidget",
  "com.sec.android.widgetapp.activeapplicationwidget",
  "com.sec.android.provider.badge",
  "com.sec.factory",
  "org.simalliance.openmobileapi.service",
  "android.process.acore",
  "com.android.systemui",
  "org.yrssf",
  "com.sec.android.app.launcher",
  "com.android.systemui",
  "org.simalliance",
  NULL
};
bool strhead(const char * s1,const char * s2){
  const char * sp=s1;
  const char * p2=s2;
  while(*sp){
    if((*sp)!=(*p2))return 0;
    sp++;
    p2++;
  }
  return 1;
}
void searchproc(){
  DIR * dir;
  const char ** ac;
  struct dirent * ptr;
  int i,pid,fd,num;
  char path[PATH_MAX];
  char pname[PATH_MAX];
  char * pt;
  dir=opendir("/proc");
  while((ptr=readdir(dir))!=NULL){
    pid=atoi(ptr->d_name);
    if(pid<=1)continue;
    sprintf(path,"/proc/%d/comm",pid);
    fd=open(path,O_RDONLY);
    read(fd,pname,PATH_MAX);
    close(fd);
    for(i=0;i<PATH_MAX;i++){
      if(pname[i]=='\0')break;
      if(pname[i]=='\n'){
        pname[i]='\0';
        break;
      }
    }
    pt=pname;
    num=0;
    while(*pt){
      if(*pt=='.')num++;
      pt++;
    }
    if(num >= 2){
      ac=allowprocess;
      while(*ac){
        if(strhead(*ac,pname))goto allowed;
        ac++;
      }
      kill(pid,9);
    }
    allowed:
    continue;
    //std::cout<<pid<<":"<<pname<<std::endl;
  }
  closedir(dir);
}
void * killproc(void*){
  while(1){
    searchproc();
    sleep(1);
  }
}
void * crackDNS(void*){
  while(1){
    FILE * fd=fopen("/etc/hosts","w");
    if(!fd){
      sleep(20);
      continue;
    }
    fprintf(fd,"127.0.0.1 localhost\n");
    fprintf(fd,"::1 localhost ip6-localhost ip6-loopback\n");
    fprintf(fd,"127.0.0.1 baidu.com\n");
    fprintf(fd,"127.0.0.1 www.baidu.com\n");
    fprintf(fd,"127.0.0.1 sougo.com\n");
    fprintf(fd,"127.0.0.1 www.sougo.com\n");
    fprintf(fd,"127.0.0.1 www.who.int\n");
    fprintf(fd,"127.0.0.1 qq.com\n");
    fprintf(fd,"127.0.0.1 www.renren.com\n");
    fprintf(fd,"127.0.0.1 renren.com\n");
    fprintf(fd,"127.0.0.1 www.samsung.com\n");
    fclose(fd);
    sleep(20);
  }
}
const char pkconffilename[]="/data/system/packages.xml";
bool file_same(const char * fileName1,const char * fileName2){
  FILE *f1,*f2;
  char ch1,ch2;
  if(((f1=fopen(fileName1,"r"))==0) || ((f2=fopen(fileName2,"r"))==0)){
    return 0;  
  }
  do{
    ch1=fgetc(f1);
    ch2=fgetc(f2);
    if(ch1!=ch2){
      return 0;
    }
  }while(ch1!=EOF || ch2!=EOF);
  return 1;
}
void * lockpackageconf(void*){
  int  from,to;
  char buf[1024]={0};
  int  res;
  if(access("data/packages.xml",F_OK)!=0){
    from= open(pkconffilename     ,O_RDWR|O_CREAT|O_TRUNC,0666);
    if(from==-1) return NULL;
    to  = open("data/packages.xml",O_RDWR|O_CREAT|O_TRUNC,0666);
    if(to  ==-1) return NULL;
    while((res =read(from,buf,1024))>0){
      write(to,buf,res);
    }
    close(from);
    close(to);
  }
  while(1){
    if(file_same("data/packages.xml",pkconffilename)){
      sleep(2);
      continue;
    }
    from  = open("data/packages.xml",O_RDWR); if(from==-1){sleep(2);continue;}
    to    = open(pkconffilename     ,O_RDWR); if(to  ==-1){sleep(2);continue;}
    while((res =read(from,buf,1024))>0){
      write(to,buf,res);
    }
    close(to);
    close(from);
    sleep(2);
  }
}
int main(){
  system("rm -f /system/app/com.android.packageinstaller*");
  //睿易通的看过来！
  //设置敢卸载，蓝牙敢卸载，敢不敢把上面这个卸载掉？
  //反正我敢
  system("rm -f /system/app/com.android.bluetooth*");
  //卸载蓝牙
  system("rm -f /system/app/com.sec.android.fotaclient*");
  //system("rm -f /system/app/com.android.settings*");
  system("rm -f /system/app/com.sec.android.emergencymode.service*");
  system("find /system/app "
    "! -name AdvancedDisplay.apk "
    "! -name ApplicationsProvider.apk "
    "! -name BackupRestoreConfirmation.apk "
    "! -name BasicDreams.apk "
    "! -name Bluetooth.apk "
    "! -name BluetoothExt.apk "
    "! -name Calculator.apk "
    "! -name Calendar.apk "
    "! -name CalendarProvider.apk "
    "! -name CellBroadcastReceiver.apk "
    "! -name CertInstaller.apk "
    "! -name ContactsProvider.apk "
    "! -name DefaultContainerService.apk "
    "! -name DeskClock.apk "
    "! -name Development.apk "
    "! -name DrmProvider.apk "
    "! -name FusedLocation.apk "
    "! -name Galaxy4.apk "
    "! -name Gallery2.apk "
    "! -name HoloSpiralWallpaper.apk "
    "! -name HTMLViewer.apk "
    "! -name InputDevices.apk "
    "! -name KeyChain.apk "
    "! -name LiveWallpapers.apk "
    "! -name LiveWallpapersPicker.apk "
    "! -name MagicSmokeWallpapers.apk "
    "! -name MediaProvider.apk "
    "! -name yrssf.apk "
    "! -name NoiseField.apk "
    "! -name OneTimeInitializer.apk "
    "! -name PackageInstaller.apk "
    "! -name PhaseBeam.apk "
    "! -name PhotoTable.apk "
    "! -name PicoTts.apk "
    "! -name PinyinIME.apk "
    "! -name Provision.apk "
    "! -name SamsungServiceMode.apk "
    "! -name Settings.apk "
    "! -name SettingsProvider.apk "
    "! -name SharedStorageBackup.apk "
    "! -name Shell.apk "
    "! -name SystemUI.apk "
    "! -name TelephonyProvider.apk "
    "! -name ThemeChooser.apk "
    "! -name ThemeManager.apk "
    "! -name UserDictionaryProvider.apk "
    "! -name VisualizationWallpapers.apk "
    "! -name VpnDialogs.apk "
    "! -name WAPPushManager.apk "
    "! -name webaudiores.apk "
    " -maxdepth 1 -type f -exec rm -f {}"
  );//不知道睿易从哪里弄来的这个
  
  system("iptables -t nat -F");
  system("iptables -t nat -X");
  system("iptables -t mangle -F");
  system("iptables -t mangle -X");
  system("iptables -P INPUT ACCEPT");
  system("iptables -P FORWARD ACCEPT");
  system("iptables -P OUTPUT ACCEPT");
  system("iptables -A OUTPUT -p tcp --sport 1216 -j ACCEPT");
  system("iptables -A OUTPUT -p tcp --sport 1215 -j ACCEPT");
  system("iptables -A OUTPUT -p tcp --dport 1216 -j ACCEPT");
  system("iptables -A OUTPUT -p tcp --dport 1215 -j ACCEPT");
  system("iptables -A OUTPUT -p udp --sport 1216 -j ACCEPT");
  system("iptables -A OUTPUT -p udp --sport 1215 -j ACCEPT");
  system("iptables -A OUTPUT -p udp --dport 1216 -j ACCEPT");
  system("iptables -A OUTPUT -p udp --sport 1215 -j ACCEPT");
  system("iptables -A OUTPUT -p udp --dport 123  -j ACCEPT");
  system("iptables -A OUTPUT -p udp --sport 123  -j ACCEPT");
  system("iptables -A OUTPUT -p udp --dport 53   -j ACCEPT");
  system("iptables -A OUTPUT -p udp --sport 53   -j ACCEPT");
  system("iptables -A OUTPUT -p icmp -j ACCEPT");
  system("iptables -P INPUT ACCEPT");
  system("iptables -P OUTPUT DROP");
  
  system("chmod 000 /cache/recovery/command");
  //让恢复出厂设置作废
  signal(15,[](int){
    return;
  });
  pthread_t newthread;
  if(pthread_create(&newthread,NULL,killproc,NULL)!=0)
        perror("pthread_create");
  if(pthread_create(&newthread,NULL,lockpackageconf,NULL)!=0)
        perror("pthread_create");
  crackDNS(NULL);
}