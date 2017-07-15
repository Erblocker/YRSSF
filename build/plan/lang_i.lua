function loaddict()
---- 语言解析器为cppjieba的lua封装版
  print("load language solver")
  langsolver.init(
    "./dict/jieba.dict.utf8",
    "./dict/hmm_model.utf8",
    "./dict/user.dict.utf8",
    "./dict/idf.utf8",
    "./dict/stop_words.utf8"
  )
end
loaddict()
print("language solver have been loaded")
