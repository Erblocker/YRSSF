dofile("./lib/base64.lua")
dofile("./lib/serialize.lua")
dofile("./lib/urlencode.lua")
local controlConfig
local userdata
function getuserdata()
  if GET["uid"]==nil then
    return false
  end
  if GET["upw"]==nil then
    return false
  end
  local uid=math.tointeger(GET["uid"])
  if uid==nil then
    return false
  end
  local upw=sqlfilter(GET["upw"])
  local res,st=runsql("select * from `user` where id="..uid)
  if st==false then
    return false
  end
  if res==nil then
    return false
  end
  if res[1]==nil then
    return false
  end
  userdata=res[1]
  return true
end
function getcontrol()
  local mode=sqlfilter(GET["mode"])
  if not mode then
    return false
  end
  local res,st=runsql("select * from `control` where name='"..mode.."'")
  if st==false then
    return false
  end
  if res==nil then
    return false
  end
  if res[1]==nil then
    return false
  end
  controlConfig=res[1]
  return true
end
function router()
  if not getcontrol() then
    return
  end
  if controlCofig[4]=="1" then
    return
  end
  if controlCofig[6]=="1" then
    local token=math.tointeger(GET["token"])
    if not uniqueExist(0,token) then
      local rdn=(math.random()*1000000000)
      rdn=math.floor(rdn)
      logUnique(0,rdn)
      RESULT="Token:"..rdn
      return
    end
  end
  if controlCofig[7]=="1" then
    if not getuserdata() then
      RESULT="Login"
      return
    end
    if controlCofig[5]=="1" then
      if not userdata[3]=="1" then
        return
      end
    end
  end
  if controlCofig[3]=="1" then
    ysThreadLock()
    dofile(controlCofig[2])
    ysThreadUnlock()
  else
    dofile(controlCofig[2])
  end
end
router()
