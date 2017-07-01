Httpd.init(Request.paser)
local header=Httpd.getheaderArray(Request.paser)
local cookie=Httpd.getcookieArray(Request.paser)
local get=Httpd.getqueryArray(Request.paser)
Httpd.readBufferBeforeSend(Request.fd)
function setusercookie(uname,pwd)
  Httpd.write(Request.fd,"Set-Cookie:uname="..uname..";path=/;")
  Httpd.write(Request.fd,"Set-Cookie:pwd="..pwd..";path=/;")
end
function getuserinfo(get)
  local keyname="user_"..get["uname"]
  local value=LDATA_read(keyname)
  if value~="" then
    return cjson.decode(value)
  end
end
function login(get)
  local user=getuserinfo(get)
  if user["pwd"]==get["pwd"] then
    setusercookie(get["uname"],user["pwd"])
  end
end
if get["mode"]=="login" then
  checkpwd(get)
end
