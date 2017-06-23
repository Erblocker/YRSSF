function getdata(name)
  return LDATA_read("passage_"..name)
end
function sres()
  RESULT=getdata(GET["pname"])
  if RESULT==nil or RESULT=="" then
    RESULT=NULL
  end
end
sres()