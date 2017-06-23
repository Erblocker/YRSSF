function controlConfig(name,arr)
  LDATA_set("config_control_"..name,cjson.encode(arr))
  print("controlConfig:"..name.."="..LDATA_read("config_control_"..name))
end
