function callAI(ttt)
  local kw=langsolver.keyword(ttt)
  local kv=langsolver.solve(ttt)
  
end
RESULT="hello"
local ttt
if type(POST)=="table" and POST["text"]~=nil then
  ttt=POST["text"]
else
  ttt=GET["text"]
end
if ttt~=nil then
  RESULT=callAI(ttt)
end