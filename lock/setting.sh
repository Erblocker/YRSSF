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
  sleep 2
done