Httpd.readBufferBeforeSend(Request.fd)
Httpd.write(Request.fd,
  "HTTP/1.0 200 OK\r\n"..
  "Content-Type:text/xml;charset=UTF-8\r\n"..
  "Cache-Control:no-cache\r\n"..
  "\r\n")
Httpd.write(Request.fd,cjson.encode(langsolver.keyword("我是拖拉机学院手扶拖拉机专业的。不用多久，我就会升职加薪，当上CEO，走上人生巅峰。")))
---- Httpd.template(Request.fd,Request.dir.."/tmp.html",{{"RESULT",RESULT}})
