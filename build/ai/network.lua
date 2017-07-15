function xnw_node_init()
  local nd={
    ["x"]   =0.0,
    ["y"]   =0.0,
    ["a"]   =0.0,
    ["b"]   =0.0,
    ["xy"]  =0.0,
    ["n"]   =0,
    ["x2"]  =0.0,
    ["next"]={}
  }
  return nd
end
function xnw_node_get(name)
  local data=LDATA_read("ai_node_"..name)
  if data=="" then
    return nil
  else
    return cjson.decode(data)
  end
end
function xnw_node_set(name,nd)
  if nd==nil then
    nd=xnw_node_init()
  end
  LDATA_set("ai_node_"..name,cjson.encode(nd))
end
function xnw_ave(x,n,w)
  return ((x*n)+w)/(n+1)
end
function xnw_node_train(nd,x,y)
  nd.x =xnw_ave(nd.x  , d.n , x)
  nd.y =xnw_ave(nd.y  , d.n , y)
  nd.xy=nd.xy+(x*y)
  nd.x2=nd.x2+(x*x)
  n=n+1
  local fm=(nd.x2-(n*nd.x*nd.x))
  local b
  if fm==0.0 then
    b=0.0
  else
    b=(nd.xy-(n*nd.x*nd.y))/fm
  end
  local a=nd.y-(b*nd.x)
  nd.b=b
  nd.a=a
  return nd
end
function xnw_node_fur(nd,x)
  return (nd.a*x)+nd.b
end
function xnw_node_fur_vec(nd,data)
  
end
function xnw_node_train_vec(nd,data,tres)
  
end
function xnw_face_train(nodes,data,tres)
  local k,v,nd
  for k,v in ipairs(nodes) do
    nd=xnw_node_get(v)
    if nd==nil then
      nd=xnw_node_init()
    end
    nd=xnw_node_train_vec(nd,data,tres)
    xnw_node_set(v,nd)
  end
end