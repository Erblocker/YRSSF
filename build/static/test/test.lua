local web={
  ["get"]={},
  ["post"]={},
  ["cookie"]={},
  ["header"]={}
}
Httpd.init(Request.paser)
web.header=Httpd.getheaderArray(Request.paser)
print("get header")
web.cookie=Httpd.getcookieArray(Request.paser)
print("get cookie")
web.get=Httpd.getqueryArray(Request.paser)
print("get query")
---- Httpd.readBufferBeforeSend(Request.fd)
Httpd.write(Request.fd,
  "HTTP/1.0 200 OK\r\n"..
  "Content-Type:text/xml;charset=UTF-8\r\n"..
  "Cache-Control:no-cache\r\n"..
  "\r\n")
Httpd.write(Request.fd,"hello\r\n")
Httpd.write(Request.fd,cjson.encode(web))