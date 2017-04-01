dofile("./lib/base64.lua")
dofile("./lib/serialize.lua")
dofile("./lib/urlencode.lua")
local umap={
  ["note"]    ="./control/note.lua",
  ["source"]  ="./control/source.lua",
  ["login"]   ="./control/login.lua",
  ["chat"]    ="./control/chat.lua",
  ["live"]    ="./control/live.lua",
}
local cont=umap[GET["mode"]]
if cont then
  local token=math.tointeger(GET["token"])
  if uniqueExist(0,token) then
    dofile(cont)
  else
    RESULT="Error:token error"
  end
else
  local rdn=(math.random()*1000000000)
  rdn=math.floor(rdn)
  logUnique(0,rdn)
  RESULT="Token:"..rdn
end