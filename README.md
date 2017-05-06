# [YRSSF](https://github.com/cgoxopx/YRSSF)  #
这是一个p2p架构的 云教学系统/直播平台框架  
## 特点： ##
*  基于UDP协议,并且可以通讯加密（虽然是使用web界面来管理，并且目前不支持https……）  
*  内置内网穿透（废话，不然怎么<u>P2P</u>）  
*  使用<u>“区块树”</u>存储数据  
*  使用“数（fang）字（mao）<u>证书</u>”来验证用户身份  
*  数据库为<u>LevelDB</u>，但是使用LUA开发时可以用`runsql`来调用<u>sqlite</u>  
*  自动根据内存池不同时间使用情况绘制曲线，然后根据曲线自动设置内存池大小（听起来高大上，实际似乎并没有什么卵用）  
*  内置锁机(#喷)（没办法，这是校内云教学软件……）  
*  操作系统兼容POSIX，比如Android,Linux。  
*  可通过web管理（<u>但是不能https</u>）  
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
## 关于Android编译： ##
本项目需要下列so文件，请自行准备
*   Tag        Type                         Name/Value
*  0x00000001 (NEEDED)                     Shared library: [libdl.so.2]
*  0x00000001 (NEEDED)                     Shared library: [libjpeg.so.8]
*  0x00000001 (NEEDED)                     Shared library: [libssl.so.1.0.0]
*  0x00000001 (NEEDED)                     Shared library: [libcrypto.so.1.0.0]
*  0x00000001 (NEEDED)                     Shared library: [libstdc++.so.6]
*  0x00000001 (NEEDED)                     Shared library: [libm.so.6]
*  0x00000001 (NEEDED)                     Shared library: [libgcc_s.so.1]
*  0x00000001 (NEEDED)                     Shared library: [libpthread.so.0]
*  0x00000001 (NEEDED)                     Shared library: [libc.so.6]