function loaduser()
  local path=APP_PATH.."/data/user.txt"
  local file=io.open(path,"r")
  local uid =hex2num(file:read("*line"))
  local pwd =file:read("*line")
  local par =file:read("*line")
  local port=file:read("*line")+0
  print("set user:"..uid)
  setClientUser(uid,pwd)
  print("set server")
  changeParentServer(par,port)
  io.close(file)
end

loaduser()
cryptModeOn()