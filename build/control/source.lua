function codefilter(str)
  local str2=string.gsub(str,"\"","")
  str2=string.gsub(str,"\\","")
  return str2
end
function download(sname,path)
  local s
  local p
  insertIntoQueue("updatekey() \n"..
  "clientDownload(\""..s.."\" , \""..p.."\") ")
end

function upload(sname,path)
  local s
  local p
  insertIntoQueue("updatekey() \n"..
  "clientUpload( \""..s.."\" , \""..p.."\" ) ")
end

function del(sname)
  local s
  insertIntoQueue("updatekey() \n"..
  "clientDownload(\""..s.."\") ")
end
local src_root=APP_PATH.."/static/mysrc/"
if GET["sname"] then
  GET["sname"]=pathfilter(GET["sname"])
  local src_path=src_root..GET["sname"]..".yss"
  if GET["swt"]=="download" then
    download(GET["sname"],src_path)
  elseif GET["swt"]=="upload" then
    upload(GET["sname"],src_path)
  elseif GET["swt"]=="delete" then
    del(GET["sname"])
  end
end
