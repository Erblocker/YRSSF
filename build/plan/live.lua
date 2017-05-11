if GLOBAL_read("live_status")=="true" then
  liveScreen()
  insertIntoQueue("dofile(\"plan/live.lua\")")
end