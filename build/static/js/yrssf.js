function openbox(arg){
  $(arg).fadeIn("10000");
}
function closebox(arg){
  $(arg).fadeOut("10000");
}
function messagebox(data){
  $("#msgbox").html(data);
  openbox("#message");
}
var user={
  "uid":0,
  "pwd":0
};
var source={
  "activity":"",
  "user"    :"",
  "sname"   :""
};
function playthemesong(){
  document.getElementById('#themesong').play();
}
function getsource(){
  $.get("ajax?gettoken=1",function(data){
    try{
      var token=data.split(":")[1];
      $.post("ajax?mode=source&activity="+source.activity+"&token="+token ,{
        "user"  :source.user,
        "sname" :source.sname
      },
      function(data){
        messageBox(data);
      });
    }catch(e){}
  });
}
