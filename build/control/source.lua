function codefilter(str)
  local str2=string.gsub(str,"\"","")
  str2=string.gsub(str,"\\","")
  return str2
end
function download(sname,path)
  local s=addslashes(sname)
  local p=addslashes(path)
  insertIntoQueue("updatekey() \n"..
  "clientDownload(\""..s.."\" , \""..p.."\") ")
end

function upload(sname,path)
  local s=addslashes(sname)
  local p=addslashes(path)
  insertIntoQueue("updatekey() \n"..
  "clientUpload( \""..s.."\" , \""..p.."\" ) ")
end

function del(sname)
  local s=addslashes(sname)
  insertIntoQueue("updatekey() \n"..
  "clientDel(\""..s.."\") ")
end
local src_root=APP_PATH.."/static/mysrc/"..getParentHash().."/"
if GET["sname"] and userdata.admin==1 then
  
  GET["sname"]=pathfilter(GET["sname"])
  
  local src_path=src_root..GET["sname"]..".yss"
  if GET["swt"]=="download" then
    
    download(GET["sname"],src_path)
    
  elseif GET["swt"]=="upload" then
    
    upload(GET["sname"],src_path)
    
  elseif GET["swt"]=="delete" then
    
    del(GET["sname"])
    os.remove(src_path)
    
  elseif GET["swt"]=="deletelocal" then
    
    os.remove(src_path)
  
  elseif GET["swt"]=="gbmode_on" then
    globalModeOn()
  elseif GET["swt"]=="gbmode_off" then
    globalModeOff()
  end
end
