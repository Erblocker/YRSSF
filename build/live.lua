if GLOBAL_read("live_status")=="true" then
  liveScreen()
  insertIntoQueue("dofile(\"live.lua\")")
end