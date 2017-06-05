Httpd.readBufferBeforeSend(Request.fd)
Httpd.write(Request.fd,
  "HTTP/1.0 200 OK\r\n"..
  "Content-Type:text/plain;charset=UTF-8\r\n"..
  "Cache-Control:no-cache\r\n"..
  "\r\n")
Httpd.write(Request.fd,"hello\r\n")
Httpd.write(Request.fd,"path:"..Request.path.."\r\n")
Httpd.write(Request.fd,"fd:"..Request.fd.."\r\n")
Httpd.write(Request.fd,"query:"..Request.query.."\r\n")
