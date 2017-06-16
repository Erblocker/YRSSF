Httpd.readBufferBeforeSend(Request.fd)
Httpd.write(Request.fd,
  "HTTP/1.0 200 OK\r\n"..
  "Content-Type:text/xml;charset=UTF-8\r\n"..
  "Cache-Control:no-cache\r\n"..
  "\r\n")
local path="./static/wmexam/wmstudyservice.WSDL/text.xml"

local file=io.open("./static/wmexam/wmstudyservice.WSDL/applist.txt","r")
local applist=file:read("*a")
file:close()
Httpd.template(Request.fd,path,{{"applist",applist}})
