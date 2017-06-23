dofile("./lib/base64.lua")
dofile("./lib/serialize.lua")
dofile("./lib/urlencode.lua")
controlConfig={}
userdata={}
function getuserdata()
  if GET["uname"]==nil then
    return false
  end
  if GET["upw"]==nil then
    return false
  end
  local uname=GET["uname"]
  if uname==nil then
    return false
  end
  local upw=GET["upw"]
  local res=cjson.decode(LDATA_read("user_"..get["uname"]))
  if res==nil then
    return false
  end
  if not res["pwd"]==GET["upw"] then
    return false
  end
  userdata=res
  return true
end
function getcontrol()
  local mode=GET["mode"]
  if not mode then
    return false
  end
  local res=cjson.decode(LDATA_read("config_control_"..mode))
  if res==nil then
    return false
  end
  controlConfig=res
  return true
end
function router()
  if GET["gettoken"]=="1" then
    local rdn=(math.random()*1000000000)
    rdn=math.floor(rdn)
    logUnique(0,rdn)
    RESULT="Token:"..rdn
    return
  end
  if not getcontrol() then
    RESULT="Empty Request"
    return
  end
  if controlConfig["disabled"]=="1" then
    return
  end
  if controlConfig["token"]=="1" then
    local token=math.tointeger(GET["token"])
    if not uniqueExist(0,token) then
      RESULT="Token Error"
      return
    end
  end
  if controlConfig["login"]=="1" then
    if not getuserdata() then
      RESULT="Login"
      return
    end
    if controlConfig["admin"]=="1" then
      if not userdata["admin"]=="1" then
        return
      end
    end
  end
  if controlConfig["unique"]=="1" then
    ysThreadLock()
    dofile(controlConfig["path"])
    ysThreadUnlock()
  else
    dofile(controlConfig["path"])
  end
end
router()
