window.onload=function(){
  try{
    playthemesong();
  }catch(e){}
  getpassage();
}
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
function chat(id){
  openbox("#chat");
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
      function(d){
        messagebox(d);
      });
    }catch(e){}
  });
}
function ubbpaser(data){
  return data;
}
var page;
var passages;
function lastpage(){
  try{
    if(page==null)return;
    if(page.length==0)return;
    getpassage(page.pop());
  }catch(e){}
}
function nextpage(){
  try{
    if(passages==null)return;
    if(passages.length==0)return;
    var endp=passages[passages.length-1];
    var data=eval("("+endp+")");
    if(data==null)return;
    if(data.name){
      getpassage(data.name);
    }
  }catch(e){}
}
function readpassage(id){
  try{
    var text=passages[id];
    var data=eval("("+text+")");
    $("#v_box_m").html(ubbpaser(data.text));
    openbox('#v_box');
  }catch(e){}
}
function sendpassage(){
  $.get("ajax?gettoken=1",function(data){
    var title=document.getElementById("ptitle").value;
    var text=$("#ptext").val();
    //alert(text);
    var name=Math.floor(Math.random()*1000000);
    try{
      var token=data.split(":")[1];
      $.post("ajax?mode=sendpassage&activity=create&token="+token+"&pname="+name ,{
        "title":title,
        "text":text
      },
      function(d){
        messagebox(d);
      });
    }catch(e){}
  });
}
function writepassage(){
  messagebox(
    "<input type='text' id='ptitle'/><br>"+
    "<textarea id='ptext' height=60% width=100%></textarea>"+
    "<br>"+
    "<a href='javascript:sendpassage()'>发送</a>"
  );
}
function getpassage(begin){
  var url="ajax?mode=viewpassage";
  if(begin){
    url+="&begin="+begin;
    page.push(begin);
  }
  $.get(url,function(data){
    //alert(data);
    var m=document.getElementById("maindiv");
    m.innerHTML="";
    try{
      var i;
      var doc=eval("("+data+")");
      passages=doc;
      for(i in doc){
        if(doc[i]){
          var el=document.createElement("div");
          try{
            var line=eval("("+doc[i]+")");
            if(line.title==null){
              continue;
            }
            var lk=document.createElement("a");
            lk.innerText=line.title;
            if(line.text==null){
              lk.href="javascript:void(0)";
            }else{
              lk.href="javascript:readpassage("+i+")";
            }
            el.appendChild(lk);
            m.appendChild(el);
          }catch(e){
            continue;
          }
        }
      }
    }catch(e){}
  });
}
function usercenter(){
  openbox("#user_center");
}
var userinfo;
function login(){
  var uname=document.getElementById("usc_login_uname").value;
  var pwd=document.getElementById("usc_login_pwd").value;
  closebox("#user_center");
  $.get("login.lua?mode=login&uname="+uname+"&pwd="+pwd,function(data){
    try{
      userinfo=eval("("+data+")");
      if(userinfo==null)return;
      if(userinfo.uname==null)return;
      closebox("#usc_login");
      openbox("#usc_info");
      $("#usc_info").html(userinfo.uname);
    }catch(e){}
  });
}