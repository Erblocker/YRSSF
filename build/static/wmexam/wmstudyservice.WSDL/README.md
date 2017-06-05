# ruiyi破解工具 #  
1.编译YRSSF  
2.下载mitmproxy `sudo apt-get install mitmproxy `  
3.下载dnsmasq   `sudo apt-get install dnsmasq `  
4.配置dnsmasq  请使用本目录的配置文件dnsmasq.conf,注意阅读此文件的最后一行  
5.重启dnsmasq `sudo service dnsmasq service`  
6.进入build目录,运行`./YRSSF`  
7.新建一个终端，运行mitmproxy `mitmproxy -P http://127.0.0.1:1215 -p 8006`  
8.打开平板电脑->wifi->dns，修改为配置dnsmasq的ip地址  
9.重启平板  
10.登录  
11.打开书包  
