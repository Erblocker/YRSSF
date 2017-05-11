if     GET["swt"]=="on" then
  liveModeOn()
elseif GET["swt"]=="off" then
  liveModeOff()
elseif GET["swt"]=="init" then
  if GET["path"] then
    boardcastScreenInit(GET["path"])
  end
elseif GET["swt"]=="close" then
  boardcastScreenDestory()
elseif GET["swt"]=="shot" then
  screenShot(APP_PATH.."static/img/"..(math.random()*100000000000)..".jpg")
elseif GET["swt"]=="boardcast" then
  boardcastScreen()
elseif GET["swt"]=="intime" then
  GLOBAL_set("live_status","true")
  insertIntoQueue("dofile(\"live.lua\")")
elseif GET["swt"]=="intimeshut" then
  GLOBAL_delete("live_status")
end
