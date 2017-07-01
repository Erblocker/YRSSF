Httpd.init(Request.paser)
local header=Httpd.getheaderArray(Request.paser)
local cookie=Httpd.getcookieArray(Request.paser)
local get=Httpd.getqueryArray(Request.paser)
Httpd.readBufferBeforeSend(Request.fd)
function setusercookie(uname,pwd)
  Httpd.write(Request.fd,"Set-Cookie:uname="..uname..";path=/;\r\n")
  Httpd.write(Request.fd,"Set-Cookie:pwd="..pwd..";path=/;\r\n")
end
function getuserinfo(get)
  local keyname="user_"..get["uname"]
  local value=LDATA_read(keyname)
  if value~="" then
    return cjson.decode(value)
  end
end
function login(get)
  if user==nil then
    return nil
  end
  local user=getuserinfo(get)
  if user["pwd"]==get["pwd"] then
    setusercookie(get["uname"],user["pwd"])
  end
  return user
end
Httpd.write(Request.fd,
  "HTTP/1.0 200 OK\r\n"..
  "Content-Type:text/xml;charset=UTF-8\r\n"..
  "Cache-Control:no-cache\r\n"
)
if get["mode"]=="login" then
  local u=login(get)
  if u["pwd"] then
    u["pwd"]=nil
    u["uname"]=get["uname"]
  end
  Httpd.write(Request.fd,
    "\r\n"..cjson.encode(u)
  )
else
  Httpd.write(Request.fd,
    "\r\nNULL"
  )
end
