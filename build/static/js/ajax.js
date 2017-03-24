$("button").click(function(){
  $.get("demo_ajax_load.txt", function(result){
    $("div").html(result);
  });
});