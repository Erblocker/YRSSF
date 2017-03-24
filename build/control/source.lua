Source={}

function Source:download(sname,path)
  updatekey()
  clientDownload(sname,path)
end

function Source:upload(sname,path)
  updatekey()
  clientUpload(sname,path)
end

function Source:del(sname)
  updatekey()
  clientDownload(sname)
end
