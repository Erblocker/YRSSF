function download(sname,path)
  updatekey()
  clientDownload(sname,path)
end

function upload(sname,path)
  updatekey()
  clientUpload(sname,path)
end

function del(sname)
  updatekey()
  clientDownload(sname)
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
