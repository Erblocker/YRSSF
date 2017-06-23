dofile("./lib/control.lua")
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
    controlConfig("live",{
        ["path"]  ="./control/live.lua",
        ["token"] =1,
        ["login"] =1,
        ["admin"] =1,
        ["unique"]=1
      }
    )
    controlConfig("connect",{
        ["path"]  ="./control/connect.lua",
        ["token"] =1,
        ["login"] =1,
        ["admin"] =1,
        ["unique"]=1
      }
    )
    controlConfig("login",{
        ["path"]  ="./control/login.lua",
        ["token"] =1,
        ["login"] =0,
        ["admin"] =0,
        ["unique"]=0
      }
    )
    controlConfig("chat",{
        ["path"]  ="./control/chat.lua",
        ["token"] =1,
        ["login"] =1,
        ["admin"] =0,
        ["unique"]=0
      }
    )
    controlConfig("source",{
        ["path"]  ="./control/source.lua",
        ["token"] =1,
        ["login"] =1,
        ["admin"] =1,
        ["unique"]=0
      }
    )
    controlConfig("viewsource",{
        ["path"]  ="./control/live.lua",
        ["token"] =0,
        ["login"] =0,
        ["admin"] =0,
        ["unique"]=0
      }
    )
    --------------------
    LDATA_set("user_admin",{
        ["pwd"]  ="",
        ["admin"]=1
      }
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