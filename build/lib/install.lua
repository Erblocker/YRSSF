function install()
  local path=APP_PATH.."/install.lock"
  local file=io.open(path,"r")
  if file==nil then
    file=io.open(path,"w")
    file:write("install")
    io.close(file)
    ----install begin
    print("install")
    --------------------
    runsql("create table `control` ("
      .."`name`     varchar(64),"
      .."`path`     varchar(1024),"
      .."`unique`   int(1) default 0,"
      .."`disabled` int(1) default 1,"
      .."`admin`    int(1) default 0,"
      .."`token`    int(1) default 1,"
      .."`login`    int(1) default 1"
      ..")"
    )
    --------------------
    runsql("create table `user` ("
      .."`id`     int key AUTO_INCREMENT,"
      .."`ysid`   varchar(16),"
      .."`admin`  int(1) default 0,"
      .."`shield` int(1) default 0,"
      .."`level`  int    default 1,"
      .."`exp`    int    default 0,"
      .."`uname`  varchar(64),"
      .."`pwd`    varchar(64),"
      .."`text`   text"
      ..")"
    )
    --------------------
    runsql("create table `source` ("
      .."`guid`    varchar(16),"
      .."`ysid`    varchar(16),"
      .."`keyword` varchar(64),"
      .."`name`    varchar(64)"
      ..")"
    )
    runsql("create table `post`("
      .."`id`      int key AUTO_INCREMENT,"
      .."`pid`     int defaule 0,"         ----父贴id
      .."`rid`     varchar(16),"           ----父资源id
      .."`uid`     int,"                   ----发帖用户
      .."`addr`    varchar(16),"           ----发帖ip
      .."`level`   int defaule 0,"         ----评论所需要的等级
      .."`keyword` varchar(64),"           ----关键词
      .."`text`    text"
      ..")"
    )
    runsql("create table `message`("
      .."`id`      int key AUTO_INCREMENT,"
      .."`text`    text,"
      .."`from`    int,"
      .."`to`      int,"
      .."`recved`  int(1)"
      ..")"
    )
    ----install end
  end
end
install()