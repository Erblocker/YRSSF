Httpd.readBufferBeforeSend(Request.fd)
Httpd.write(Request.fd,
  "HTTP/1.0 200 OK\r\n"..
  "Content-Type:text/xml;charset=UTF-8\r\n"..
  "Cache-Control:no-cache\r\n"..
  "\r\n")
local path="./static/wmexam/wmstudyservice.WSDL/text.xml"
print(path)
local file=io.open(path,"r")
local mark=file:read(0)
  local pbk
  while mark do
    pbk =file:read("*line")
    if pbk then
      Httpd.write(Request.fd,pbk)
    end
    mark=file:read(0)
    if not mark then
      break
    end
  end
io.close(file)