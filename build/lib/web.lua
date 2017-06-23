web={
  ["req"]    =nil,
  ["get"]    ={},
  ["post"]   ={},
  ["cookie"] ={},
  ["header"] ={},
  ["req"]    ={}
}
function web.common(req)
  local o={}
  setmetatable(o,web)
  o.req=req
  Httpd.init(req.paser)
  web.header=Httpd.getheaderArray(req.paser)
  web.cookie=Httpd.getcookieArray(req.paser)
  web.get=Httpd.getqueryArray(req.paser)
  
  if Httpd.checkpost(req.paser) then
    Httpd.getpostArray(req.paser)
  else
    Httpd.readBufferBeforeSend(req.fd)
  end
  
  return o
end
function web:echo(str)
  Httpd.write(self.req.fd,str)
end
function web:ok()
  self:echo(
  "HTTP/1.0 200 OK\r\n"..
  "Content-Type:text/html;charset=UTF-8\r\n"..
  "Cache-Control:no-cache\r\n"..
  "\r\n")
end
function web:template(path,assign)
  Httpd.template(self.req.fd,path,assign)
end
