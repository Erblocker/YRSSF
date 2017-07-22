function setkw(kw)
  if kw==nil then
    return
  end
--  if type(kw)~="table" then
--    return
--  end
  local k,v
  local word,acl,comm,arr
  local dbk
  for k,v in ipairs(kw) do
    word=v[1]
    acl=v[2]
    comm={
      ["mode"]    ="mem",
      ["exp"]     =acl,
      ["location"]=GET["pname"]
    }
    dbk="ai_mem_text_"..word
    arr=cjson.decode(LDATA_read(dbk))
    if type(arr)=="table" then
      table.insert(arr,comm)
    else
      arr={comm}
    end
    LDATA_set(dbk,cjson.encode(arr))
  end
end
function createdata(arr)
  local ori=LDATA_get("passage_"..GET["pname"])
  if ori=="" or ori==nil then
    LDATA_set("passage_"..GET["pname"],cjson.encode(arr))
  end
end
function deletedata(name)
  LDATA_delete("passage_"..name)
end
function sres()
  if GET["activity"]=="create" then
    local keyword=langsolver.keyword(POST["text"])
    setkw(keyword)
    createdata({
        ["text"]   =POST["text"],
        ["title"]  =POST["title"],
        ["name"]   =GET["pname"],
        ["keyword"]=keyword,
        ["time"]   =nil
      }
    )
    RESULT="Success"
    return
  end
  if GET["activity"]=="delete" then
    deletedata(GET["pname"])
    RESULT="Success"
    return
  end
end
  if userdata.post==1 then
    sres()
  else
    RESULT="Permission denied"
  end
