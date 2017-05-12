if GLOBAL_read("live_status")=="true" then
  liveScreen()
  boardcastSound()
  insertIntoQueue("dofile(\"plan/live.lua\")")
end