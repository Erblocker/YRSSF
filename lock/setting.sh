while [ "1" = "1" ];
do
  echo "nameserver 127.0.0.1" > /etc/resolv.conf
  sleep 2
done