web={
  ["get"]="",
  ["post"]="",
  ["cookie"]="",
  ["header"]=""
}
Httpd.init(Request.paser)
web.header=Httpd.getheaderArray(Request.paser)
web.cookie=Httpd.getcookieArray(Request.paser)
web.get=Httpd.getqueryArray(Request.paser)
Httpd.readBufferBeforeSend(Request.fd)
Httpd.write(Request.fd,
  "HTTP/1.0 200 OK\r\n"..
  "Content-Type:text/xml;charset=UTF-8\r\n"..
  "Cache-Control:no-cache\r\n"..
  "\r\n")
Httpd.write(Request.fd,"hello\r\n")
Httpd.write(Request.fd,web.get["id"])