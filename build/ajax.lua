---- yrssf的重要入口点之一
---- 在http://0.0.0.0/ajax 被访问时此程序会被调用
---- 系统变量：
------ (const table)GET      //get方式接受的数据
------ (const table)POST     //post方式接受的数据
------ (const table)COOKIE   //cookies不解释
------ (string)     RESULT   //将返回結果賦值給此變量
dofile("./lib/base64.lua")
dofile("./lib/serialize.lua")
dofile("./lib/urlencode.lua")
controlConfig={}
userdata={}
function getuserdata()
  
  local uname
  local upw
  
  if GET["uname"]~=nil then
    uname=GET["uname"]
  else
    if COOKIE["uname"]~=nil then
      uname=COOKIE["uname"]
    else
      return false
    end
  end
  
  if GET["upw"]~=nil then
    uname=GET["upw"]
  else
    if COOKIE["pwd"]~=nil then
      uname=COOKIE["pwd"]
    else
      return false
    end
  end
  
  if uname==nil then
    return false
  end
  
  local res=cjson.decode(LDATA_read("user_"..get["uname"]))
  
  if res==nil then
    return false
  end
  
  if res=="" then
    return false
  end
  if res["pwd"]~=GET["upw"] then
    return false
  end
  userdata=res
  
  userdata["uname"]=uname
  
  return true
end
function getcontrol()
  local mode=GET["mode"]
  if (not mode) then
    return false
  end
  local res=cjson.decode(LDATA_read("config_control_"..mode))
  if res==nil then
    return false
  end
  if res=="" then
    return false
  end
  controlConfig=res
  return true
end
function router()
  if GET["getuserinfo"]=="1" then
    
    ---- print(cjson.encode(COOKIE))
    ---- print(cjson.encode(HEADER))
    
    if (not getuserdata()) then
      RESULT="NULL"
      return
    end
    if type(userdata)=="table" then
      RESULT=cjson.encode(userdata)
    end
    return
  end
  if GET["gettoken"]=="1" then
    local rdn=(math.random()*1000000000)
    rdn=math.floor(rdn)
    logUnique(0,rdn)
    RESULT="Token:"..rdn
    return
  end
  if (not getcontrol()) then
    RESULT="Empty Request"
    return
  end
  if controlConfig["disabled"]==1 then
    return
  end
  if controlConfig["token"]==1 then
    local token=math.tointeger(GET["token"])
    if (not uniqueExist(0,token)) then
      RESULT="Token Error"
      return
    end
  end
  if controlConfig["login"]==1 then
    if (not getuserdata()) or userdata["uname"]==nil then
      RESULT="Login"
      return
    end
    if controlConfig["admin"]==1 then
      if userdata["admin"]~=1 then
        return
      end
    end
  end
  if controlConfig["unique"]==1 then
    ysThreadLock()
    dofile(controlConfig["path"])
    ysThreadUnlock()
  else
    dofile(controlConfig["path"])
  end
end
router()
