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
    controlConfig("viewpassage",{
        ["path"]  ="./control/viewpassage.lua",
        ["token"] =0,
        ["login"] =0,
        ["admin"] =0,
        ["unique"]=0
      }
    )
    controlConfig("sendpassage",{
        ["path"]  ="./control/sendpassage.lua",
        ["token"] =1,
        ["login"] =1,
        ["admin"] =0,
        ["unique"]=0
      }
    )
    controlConfig("ai",{
        ["path"]  ="./control/ai.lua",
        ["token"] =0,
        ["login"] =0,
        ["admin"] =0,
        ["unique"]=0
      }
    )
    --------------------
    LDATA_set("user_admin",{
        ["pwd"]  ="",
        ["admin"]=1,
        ["post"] =1
      }
    )
    LDATA_set("passage",'NULL')
    LDATA_set("passage_passage1",'{"title":"第一篇文章","text":"test1","name":"passage1"}')
    LDATA_set("passage_passage2",'{"title":"第二篇文章","text":"test2","name":"passage2"}')
    --------------------
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