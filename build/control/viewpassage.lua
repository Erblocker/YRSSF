function sres()
  RESULT=cjson.encode(LDATA_read("passage_"..GET["pname"]))
  if RESULT==nil or RESULT=="" then
    RESULT=NULL
  end
end

function getall()
  local bg
  if GET["begin"]==nil or GET["begin"]=="" then
    bg="passage"
  else
    bg="passage_"..GET["begin"]
  end
  RESULT=cjson.encode(LDATA_find(bg,0,40,"passage"))
  if RESULT==nil or RESULT=="" then
    RESULT=NULL
  end
end

if GET["pname"]==nil or GET["pname"]=="" then
  getall()
else
  sres()
end