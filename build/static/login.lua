Httpd.init(Request.paser)
local header=Httpd.getheaderArray(Request.paser)
local cookie=Httpd.getcookieArray(Request.paser)
local get=Httpd.getqueryArray(Request.paser)
Httpd.readBufferBeforeSend(Request.fd)
function setusercookie(uname,pwd)
  Httpd.write(Request.fd,"Set-Cookie:uname="..uname..";path=/;")
  Httpd.write(Request.fd,"Set-Cookie:pwd="..pwd..";path=/;")
end
