function setkw(kw)
  if kw==nil then
    return
  end
end
fuction createdata(name,arr)
  LDATA_set("passage_"..name,cjson.encode(arr))
end
fuction deletedata(name)
  LDATA_delete("passage_"..name)
end
function sres()
  if not userdata["post"]==1 then
    RESULT="Permission denied"
    return
  end
  if GET["activity"]=="create" then
    local keyword=langsolver.keyword(POST["text"])
    setkw(keyword)
    createdata(GET["pname"],{
        ["text"]   =POST["text"],
        ["title"]  =POST["title"],
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
sres()