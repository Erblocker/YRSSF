function connresolver()
  if GET["activity"]=="golast" then
  ----回到上一个peer或者node
    goLastServer()
    RESULT="succeed"
    return
  end
  if GET["activity"]=="getcert" then
    
    ----获取数字证书
    clientGetUniKey()
    
    return
  end
  if GET["activity"]=="conn" then
    if GET["uid"]~=nil then
      
      ----建立p2p连接
      RESULT=cjson.encode(connectToUser(math.tointeger(GET["uid"])))
      
      ----关掉系统默认加密
      ----因为连接时会自动使用ECC加密
      cryptModeOff()
      
      ----确认数字证书
      ----数字证书生成后需要立刻使用，因为有效期非常短
      clientRegister()
      
      ----登录至peer
      clientLogin()
      
      cryptModeOff()
      return
    end
    return
  end
end
connresolver()
