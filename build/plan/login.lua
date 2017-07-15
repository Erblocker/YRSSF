function loaduser()
  local path=APP_PATH.."/data/user.txt"
  local file=io.open(path,"r")
  local uid =hex2num(file:read("*line"))
  local pwd =file:read("*line")
  local par =file:read("*line")
  local port=file:read("*line")+0
  print("set user:"..uid)
  changeParentServer(par,port)
  setClientUser(uid,pwd)
  ---- updatekey()
  ---- 好像没什么用
  serverUpdateKey()
  setServerUser(uid,pwd)
  print("set server")
  io.close(file)
end

checkSignOn()
----把上面的On改成Off，程序将不会验证签名
----注意：此操作有安全隐患
----建议不要进行此操作
----本程序使用的公钥又不给钱，何苦呢？

loaduser()
cryptModeOn()
