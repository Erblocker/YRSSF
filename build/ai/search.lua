function ai_search_exp(arr)
  local k,v,word,exp,res,rk,rv
  local st={}
  ---- 统计表
  for k,v in ipairs(arr) do
    word=v[1]
    exp =v[2]
    res=cjson.decode(LDATA_read("ai_mem_text_"..word))
    if type(res)=="table" then
      for rk,rv in ipairs(res) do
        if st[rv.location]==nil then
          st[rv.location]=0.0
        else
          st[rv.location]=st[rv.location]+rv.exp
        end
      end
    end
  end
  return st
end
function ai_search_arr(arr)
  local exps=ai_search_exp(arr)
  ---- 获取权重列表
end
function ai_search_word(str)
  return ai_search_arr(langsolver.keyword(str))
end
