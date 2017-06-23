fuction createdata(name,arr)
  LDATA_set("viewsource_"..name,cjson.encode(arr))
end
fuction getdata(name)
  LDATA_read("viewsource_"..name)
end
fuction deletedata(name)
  LDATA_delete("viewsource_"..name)
end