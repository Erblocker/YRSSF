dofile("./lib/base64.lua")
dofile("./lib/serialize.lua")
dofile("./lib/urlencode.lua")
local controlConfig
local userdata
function getuserdata()
  if GET["uname"]==nil then
    return false
  end
  if GET["upw"]==nil then
    return false
  end
  local uname=math.tointeger(GET["uname"])
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
  if controlCofig["disabled"]=="1" then
    return
  end
  if controlCofig["token"]=="1" then
    local token=math.tointeger(GET["token"])
    if not uniqueExist(0,token) then
      RESULT="Token Error"
      return
    end
  end
  if controlCofig["login"]=="1" then
    if not getuserdata() then
      RESULT="Login"
      return
    end
    if controlCofig["admin"]=="1" then
      if not userdata["admin"]=="1" then
        return
      end
    end
  end
  if controlCofig["unique"]=="1" then
    ysThreadLock()
    dofile(controlCofig["path"])
    ysThreadUnlock()
  else
    dofile(controlCofig["path"])
  end
end
router()
