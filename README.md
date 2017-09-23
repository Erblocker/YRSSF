# [YRSSF](https://github.com/cgoxopx/YRSSF)  #
YRSSF（Yun'er Study System Framework 或者 Shuang Si Ruiyi Famework），这是一个p2p架构的 云教学系统/直播平台框架（CMS），目的是开发一个[睿易云](http://www.ruiyiyun.com)的免费替代品。  
本项目遵守GPL-3.0协议  
如需用于商业用途等需要改协议，请联系qq  
*  2577199574  
*  3068750058  
## 特点： ##
*  基于UDP协议,并且可以通讯加密（虽然是使用web界面来管理，并且目前不支持https……）  
*  内置内网穿透（废话，不然怎么<u>P2P</u>）  
*  使用<u>“区块树”</u>存储数据  
*  使用“数（fang）字（mao）<u>证书</u>”来验证用户身份  
*  数据库为<u>LevelDB</u>，但是使用LUA开发时可以用`runsql`来调用<u>sqlite</u>  
*  内置锁机(#喷)（没办法，这是校内云教学软件……）  
*  操作系统兼容POSIX，比如Android,Linux。  
*  可通过web管理（<u>但是不能https</u>）  
*  支持cgi和fastcgi（可以直接移植php项目什么的了）  
*  目前尚不支持Windows，可通过VNC来使用Windows程序  
## 文档：  ##  
[APIs](build)  
## 警告：  ##  
lock目录下的东西极度危险！极度危险！极度危险！重要的事说三遍  
经测试，在原版linux下可导致[百度](https://www.baidu.com)等常见网站DNS解析错误，并且会自动关闭大多数端口  
在android下会同时卸载包括<u>蓝牙</u>在内的大多数系统app，并且结束未允许的用户app的进程（具体效果见睿易派）  
如果不需要限制设备功能（比如说防止学生用平板电脑打游戏……），请不要在lock目录里面乱make，否则后果很严重……  
如果真的make了，在里面`make clean`一下可以解除<u>锁机</u>效果  
此工具请勿用于非法用途，否则后果自负  
## 编译： ##  
### Linux: ###  
#### 下载并编译：  
` $ git clone https://github.com/cgoxopx/YRSSF `  
或者是` $ git clone http://git.oschina.net/cgoxopx/YRSSF `(如果网速慢的话)  
` $ cd YRSSF && make `  
安装人工智能组件(可省略，因为一般人用不到，而且atulocher项目进展非常缓慢，并且可能会用到一些硬件特性)  
` $ make atulocher`  
` $ cd core  && make `  
` $ cd ../launcher && make `  
` $ cd ../build `  
#### 让系统重新生成密钥：  
` $ rm data/cert.txt `生成后第一行是公钥，第二行是私钥  
#### 重新生成数据库：  
` $ rm data/yrssf.db`  
#### 添加信任的公钥：  
` $ echo "public key" >> data/allowcert.txt`  
public key 换成可能要连接的服务器公钥  

#### 设置用户名：  
` $ vim data/user.txt`  
#### 最后运行项目：  
` $ ./launcher `  
#### 结束进程（如果需要的话）：  
` $ killall YRSSF launcher daemon`  
launcher和daemon会相互保护，单独结束其中任意一个都会被另一个复活  
### Android编译： ###  
请直接使用cmake  
## 插件开发： ##  
编译完成后，/build 整个目录可以直接复制出来在其他位置执行  
/build/static 为www目录（存放网页以及cgi文件）  
/build/live   下的文件为直播的管道文件  
/build/static 目录支持cgi文件或者php文件，但是使用cgi文件时请注意：如果cgi程序有错误，http服务器关闭时会直接`Segmentation fault`（这似乎是cgi的通病）  
# 鸣谢 #   
*  lua  
*  lua-cjson  
*  leveldb  
*  zlib  
*  sqlite  
*  cppjieba  
*  [studyHttpd](https://github.com/tw1996/studyHttpd)  
*  tinyhttpd  
*  SDL  
***************
最后还是要感谢[睿易派](http://www.ruiyiyun.com)  