dofile("./lib/base64.lua")
dofile("./lib/serialize.lua")
dofile("./lib/install.lua")
print("initing")
print("path:"..APP_PATH)
function loadcert()
  local path="./data/cert.txt"
  local file=io.open(path,"r")
  local certdata
  if file then
    local publickey =ZZBase64.decode(file:read("*line"))
    local privatekey=file:read("*line")
    signerInit(publickey,privatekey)
    io.close(file)
  else
    file=io.open(path,"w")
    local a,b=signerInit()
    a=ZZBase64.encode(a)
    file:write(a.."\n"..b)
    io.close(file)
  end
end

function loadAllowCerts()
  local path="./data/allowcert.txt"
  local file=io.open(path,"r")
  if file==nil then
    return nil
  end
  print("load cert list")
  local mark=file:read(0)
  local pbk
  while mark do
    pbk =file:read("*line")
    if pbk then
      addSignKey(ZZBase64.decode(pbk))
    end
    mark=file:read(0)
    if not mark then
      break
    end
  end
  io.close(file)
end
loadcert()
loadAllowCerts()
insertIntoQueue("dofile(\"plan/login.lua\")")

dofile("./init/httpd.lua")

print("inited")
