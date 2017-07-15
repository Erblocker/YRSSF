window.onload=function(){
  try{
    playthemesong();
  }catch(e){}
  getpassage();
  updateuseri();
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
      $.post("ajax?mode=source&swt="+source.activity+"&token="+token+"&sname="+source.sname ,{
        "user"  :source.user
      },
      function(d){
        messagebox(d);
      });
    }catch(e){}
  });
}
function ubbpaser(data){
  var str=data;
  str=str.replace("<","&lt;");
  str=str.replace(">","&gt;");
  str=str.replace("'","&#39;");
  str=str.replace('"',"&#34;");
  str=str.replace(/(http:\/\/|https:\/\/)((\w|=|\?|\.|\/|&|-)+)/g , "<a href='$1$2'>$1$2</a>");
  return str;
}
var page=[];
var passages;
function lastpage(){
    if(page==null || page.length==0 || page[0]==null){
      getpassage();
    }else{
      var beg=page.pop();
      alert(beg);
      getpassage(beg);
    }
}
function nextpage(){
    if(passages==null)return;
    if(passages.length==0)return;
    var endp=passages[passages.length-1];
    var data=eval("("+endp+")");
    if(data==null)return;
    if(data.name){
      getpassage(data.name);
    }
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
  if(begin!=null){
    url+="&begin="+begin;
    page.push(begin);
  }
  $.get(url,function(data){
    //alert(data);
    var m=document.getElementById("maindiv");
    if(page.length!=0){
      m.innerHTML=("<br><a href='javascript:lastpage();'>上一页</a>");
    }else{
      m.innerHTML="";
    }
    try{
      var i,j=0,k=0;
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
            j++;
            if(j==1 && page.length!=0){
              continue;
            }
            k++;
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
      if(k==0){
        page.pop();
        return;
      }
      $("#maindiv").append("<br><a href='javascript:nextpage();'>更多...</a>");
    }catch(e){}
  });
}
function usercenter(){
  openbox("#user_center");
}
var userinfo;
function updateuseri(){
  $.get("ajax?getuserinfo=1",function(data_dd){
    try{
      var data=eval("("+data_dd+")");
      if(data==null)return;
      if(data.uname==null)return;
      userinfo=data;
      openbox("#usc_info");
      closebox("#usc_login");
      $("#usc_info").html(userinfo.name);
      if(userinfo.admin==1){
        $("#usc_info").append("<br><span style='color:#f00;'>管理员</span>");
        $("#usc_info").append("<br><a href='javascript:updatesrclist()'>更新资源</a>");
        $("#usc_info").append("<br><a href='javascript:showsrclist()'>资源列表</a>");
      }
    }catch(e){}
  });
}
function login(){
  var uname=document.getElementById("usc_login_uname").value;
  var pwd=document.getElementById("usc_login_pwd").value;
  document.cookie="uname="+escape(uname) +";";
  document.cookie="pwd="+  escape(pwd)   +";";
  closebox("#user_center");
  updateuseri();
}
function updatesrclist(){
  source.activity="download";
  source.sname="sourceli";
  $.get("ajax?gettoken=1",function(data){
    try{
      var token=data.split(":")[1];
      $.get("ajax?mode=&swt=gbmode_on&sname=0&token="+token,function(){
        getsource();
      });
    }catch(e){}
  });
}
function showsrclist(){
  $.get("mysource/sourceli.yss",function(data){
    $("#srcbox").html(data);
    openbox("#source");
  });
}
var gettingsrc;
function getsrc(srcname){
  gettingsrc=srcname;
  $.ajax({
    'url':    'mysrc/'+gettingsrc+'.yss',
    'type':   'GET',
    'success':function(data){
      messagebox(data);
    },
    'error':  function(){
      $.ajax({
        'url':    'srcs/'+gettingsrc+'.yss',
        'type':   'GET',
        'success':function(data){
          messagebox(data);
        },
        'error':  function(){
          source.activity="download";
          source.sname=gettingsrc;
          getsource();
          alert("正在下载资源，请稍后……");
        }
      });
    }
  });
}
function delsrc(srcname){
  source.activity="delete";
  source.sname=srcname;
  getsource();
}
function delLocalsrc(srcname){
  source.activity="deletelocal";
  source.sname=srcname;
  getsource();
}